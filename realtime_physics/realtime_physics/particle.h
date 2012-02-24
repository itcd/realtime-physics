#ifndef PARTICLE_H
#define PARTICLE_H

struct Particle
{
	float mass;
	glm::vec3 position;
	glm::vec3 velocity;
	float radius;
	float lifetime;
	float age;
	bool alive;

	Particle(float mass, glm::vec3 position, glm::vec3 velocity, float radius = 0.025, float lifetime = 1)
	{
		this->mass = mass;
		this->position = position;
		this->velocity = velocity;
		this->radius = radius;
		this->lifetime = lifetime;
		this->age = 0;
		this->alive = true;
	}

	void update(float timestep = 0.005)
	{
		// use the mid-point method for integration
		auto x0 = position;
		auto v0 = velocity;

		// get position and velocity at the mid-point
		float time_half = timestep / 2;
		position += velocity * time_half;
		velocity += force() / mass * time_half;

		// estimate the final position and velocity
		position = x0 + velocity * timestep;
		velocity = v0 + force() / mass * timestep;

		// increase the particle's age
		age += timestep;

		// contract a particle if it is going to die
		const float epsilon = 0.0001;
		float threshold = lifetime - radius / epsilon * timestep;
		if (age > threshold && radius > epsilon)
		{
			radius -= epsilon;
		}
	}

	bool is_alive()
	{
		return age < lifetime && alive;
	}

	glm::vec3 force()
	{
		return gravity() + viscous_drag() + random_force();
	}

	glm::vec3 gravity()
	{
		return mass * 9.8f * glm::vec3(0, -1, 0);
	}

	glm::vec3 viscous_drag()
	{
		return -0.1f * velocity;
	}

	glm::vec3 random_force()
	{
		return random_vec3() * 10.f;
	}
};

#endif // PARTICLE_H
