#ifndef RIGID_BODY_H
#define RIGID_BODY_H

#include "geometry.h"
#include "distance.h"

struct RigidBody
{
	glm::vec3 position;
	glm::vec3 velocity;
	glm::quat orientation;
	glm::vec3 angular_velocity;
	//glm::vec3 linearMomentum;
	//glm::vec3 angularMomentum;
	std::shared_ptr<Geometry> geometry;
	float lifetime;
	float age;
	glm::vec3 position_old;

	RigidBody(glm::vec3 position, glm::vec3 velocity, glm::quat orientation, glm::vec3 angular_velocity, std::shared_ptr<Geometry> geometry, float lifetime = 10)
	{
		this->position_old = this->position = position;
		this->velocity = velocity;
		this->orientation = orientation;
		this->angular_velocity = angular_velocity;
		this->geometry = geometry;
		this->lifetime = lifetime;
		this->age = 0;
	}

	glm::vec3 force()
	{
		if (enable_gravity())
		{
			return gravity() + viscous_drag() + random_force();
		} 
		else
		{
			return glm::vec3(0, 0, 0);
		}
	}

	glm::vec3 gravity()
	{
		return geometry->mass() * 9.8f * glm::vec3(0, -1, 0);
	}

	glm::vec3 viscous_drag()
	{
		return -0.1f * velocity;
	}

	glm::vec3 random_force()
	{
		return random_vec3() * 10.f;
	}

	glm::vec3 torque()
	{
		return glm::vec3(0, 0, 0);
	}

	bool is_alive()
	{
		return age < lifetime;
	}

	void update(float timestep = 0.005)
	{
		position_old = position;

		// use the mid-point method for integration
		auto x0 = position;
		auto v0 = velocity;

		// get position and velocity at the mid-point
		float time_half = timestep / 2;
		position += velocity * time_half;
		velocity += force() / geometry->mass() * time_half;

		// estimate the final position and velocity
		position = x0 + velocity * timestep;
		velocity = v0 + force() / geometry->mass() * timestep;

		// rotate it according to the angular velocity
		auto angle = glm::degrees(glm::length(angular_velocity) * timestep);
		orientation = glm::rotate(orientation, angle, angular_velocity);

		// whether a rigid body will disappear after it run out of its life
#if 0
		// increase its age
		age += timestep;
#endif
	}

	void draw()
	{
		glPushMatrix();
		glTranslated(position.x, position.y, position.z);
		nv::applyRotation(nv::quaternionf(orientation.w, orientation.x, orientation.y, orientation.z));
		geometry->operator()();
		geometry->draw_sphere();
		geometry->draw_AABB();
		glPopMatrix();
	}

	bool is_on_the_other_side(glm::vec3 x, glm::vec3 point, glm::vec3 normal)
	{
		float result = glm::dot((x - point), normal);
		return result < 0;
	}

	glm::vec3 velocity_normal(const RigidBody &rb)
	{
		auto normal = glm::normalize(rb.position - position);
		return glm::dot(normal, velocity) * normal;
	}

	void respond(RigidBody &rb)
	{
		auto u1 = velocity_normal(rb);
		auto v_tangential = velocity - u1;
		auto u2 = rb.velocity_normal(*this);
		auto v_tangential_rb = rb.velocity - u2;
		auto m1 = geometry->mass();
		auto m2 = rb.geometry->mass();

		// Conservation of linear momentum
		auto v1 = (m1-m2)/(m1+m2)*u1 + 2*m2/(m1+m2)*u2;
		auto v2 = (m2-m1)/(m1+m2)*u2 + 2*m1/(m1+m2)*u1;
		velocity = v1 + v_tangential;
		rb.velocity = v2 + v_tangential_rb;

		auto d = glm::distance(position, rb.position);
		auto distance = geometry->radius() + rb.geometry->radius();
		if (d - distance < EPLISON)
		{
			auto diff = abs(distance - d) + EPLISON * 2;
			auto normal = glm::normalize(position - rb.position);
			auto mass = geometry->mass() + rb.geometry->mass();
			auto weight1 = geometry->mass() / mass;
			auto weight2 = rb.geometry->mass() / mass;
			position += weight1 * diff * normal;
			rb.position += weight2 * diff * (-normal);
		}
	}

	void collide(RigidBody &rb)
	{
		auto d = closest_distance(position_old, position, rb.position_old, rb.position);
		if (d <= geometry->radius() + rb.geometry->radius())
		{
			respond(rb);
		}
	}
};

#endif // RIGID_BODY_H
