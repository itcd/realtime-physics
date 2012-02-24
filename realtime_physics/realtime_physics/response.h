#ifndef RESPONSE_H
#define RESPONSE_H

#include "plane.h"
#include "rigid_body.h"

void rb_plane_response_naive(RigidBody &rb, Plane plane)
{
	auto vertices = rb.geometry->vertices(rb.orientation, rb.position);
	if (vertices.size() <= 0)
	{
		return;
	}

	auto min_vertex = vertices[0];
	auto min = glm::dot(vertices[0] - plane.point, plane.normal);
	for (auto i=vertices.begin(); i!=vertices.end(); i++)
	{
		auto d = glm::dot(*i - plane.point, plane.normal);
		if (d < min)
		{
			min = d;
			min_vertex = *i;
		}
	}
	if (min <= 0)
	{
		plane.collide_at_position(rb, min_vertex);
	}
}

void rb_plane_response(RigidBody &rb1, Plane plane)
{
	auto vertices = rb1.geometry->vertices(rb1.orientation, rb1.position);
	if (vertices.size() <= 0)
	{
		return;
	}

	std::vector<glm::vec3> vertex_list;
	glm::vec3 min_vertex = vertices[0];
	auto min = glm::dot(vertices[0] - plane.point, plane.normal);
	for (auto i=vertices.begin(); i!=vertices.end(); i++)
	{
		auto d = glm::dot(*i - plane.point, plane.normal);
		if (d < min)
		{
			min = d;
			min_vertex = *i;
		}
		//if (d < EPLISON)
		//{
		//	vertex_list.push_back(*i);
		//}
	}
	vertex_list.push_back(min_vertex);

	if (min < EPLISON)
	{
		auto a = rb1.position;
		auto normal = plane.normal;

		for (auto i=vertex_list.begin(); i!=vertex_list.end(); i++)
		{
			// compute linear velocity and relative velocity
			auto p_a = *i;
			auto r_a = p_a - a;
			auto v_a = rb1.velocity + glm::cross(rb1.angular_velocity, r_a);
			auto v_b = glm::vec3(0, 0, 0);
			auto v_rel = glm::dot(normal, v_a - v_b);

			// compute impulse magnitude j
			auto epison = plane.bounce_factor;
			auto N = -(1 + epison) * v_rel;
			auto t1 = 1 / rb1.geometry->mass();
			auto t2 = 0;
			auto a_r = glm::mat3_cast(rb1.orientation);
			auto a_r_t = glm::transpose(a_r);
			auto a_inv = (a_r * glm::inverse(rb1.geometry->inertial_tensor()) * a_r_t) * glm::cross(r_a, normal);
			auto t3 = glm::dot(normal, glm::cross(a_inv, r_a));
			auto t4 = 0;
			auto j = N / (t1 + t2 + t3 + t4);
			auto J_n = j * normal;

			// update linear velocity and angular velocity
			rb1.velocity += J_n / rb1.geometry->mass();
			//rb1.angular_velocity += glm::inverse(rb1.geometry->inertial_tensor()) * glm::cross(r_a, J_n);
		}

		// post processing, if the two objects overlap, separate them
		auto diff = abs(min) + EPLISON;
		rb1.position += diff * normal;
	}
}

void rb_rb_response(RigidBody &rb1, RigidBody &rb2, glm::vec3 normal)
{
	auto a = rb1.position;
	auto b = rb2.position;
	auto ab = b - a;
	auto ba = a - b;

	//// normal approach 1
	//// these are not the exact colliding points and normal, just a simple approximation
	//normal = glm::normalize(ba);

	// normal approach 2
	auto dist = glm::length(normal);
	normal = glm::normalize(normal);

	auto p_a = get_farthest_point_in_direction(ab, rb1.geometry->vertices(rb1.orientation, rb1.position));
	auto p_b = get_farthest_point_in_direction(ba, rb2.geometry->vertices(rb2.orientation, rb2.position));

	// compute linear velocity and relative velocity
	auto r_a = p_a - a;
	auto r_b = p_b - b;
	auto v_a = rb1.velocity + glm::cross(rb1.angular_velocity, r_a);
	auto v_b = rb2.velocity + glm::cross(rb2.angular_velocity, r_b);
	auto v_rel = glm::dot(normal, v_a - v_b);

	// compute impulse magnitude j
	auto epison = 1;
	auto N = -(1 + epison) * v_rel;
	auto t1 = 1 / rb1.geometry->mass();
	auto t2 = 1 / rb2.geometry->mass();
	auto a_r = glm::mat3_cast(rb1.orientation);
	auto a_r_t = glm::transpose(a_r);
	auto a_inv = (a_r * glm::inverse(rb1.geometry->inertial_tensor()) * a_r_t) * glm::cross(r_a, normal);
	auto t3 = glm::dot(normal, glm::cross(a_inv, r_a));
	auto b_r = glm::mat3_cast(rb2.orientation);
	auto b_r_t = glm::transpose(b_r);
	auto b_inv = (b_r * glm::inverse(rb2.geometry->inertial_tensor()) * b_r_t) * glm::cross(r_b, normal);
	auto t4 = glm::dot(normal, glm::cross(b_inv, r_b));
	auto j = N / (t1 + t2 + t3 + t4);
	auto J_n = j * normal;

	// update linear velocity and angular velocity
	rb1.velocity += J_n / rb1.geometry->mass();
	rb1.angular_velocity += glm::inverse(rb1.geometry->inertial_tensor()) * glm::cross(r_a, J_n);
	rb2.velocity += -J_n / rb2.geometry->mass();
	rb2.angular_velocity += glm::inverse(rb2.geometry->inertial_tensor()) * glm::cross(r_b, -J_n);

	// non-inter-penetration constraint
	// if the two objects overlap, separate them
	auto d_a = glm::dot(p_a - b, normal);
	auto d_b = glm::dot(p_b - b, normal);
	if (d_a - d_b < EPLISON)
	{
		auto diff = abs(d_b - d_a) + 2 * EPLISON;
		//auto diff = dist + 2 * EPLISON;
		auto mass = rb1.geometry->mass() + rb2.geometry->mass();
		auto weight1 = rb1.geometry->mass() / mass;
		auto weight2 = rb2.geometry->mass() / mass;
		rb1.position += weight1 * diff * normal;
		rb2.position += weight2 * diff * (-normal);
	}
}

#endif // RESPONSE_H
