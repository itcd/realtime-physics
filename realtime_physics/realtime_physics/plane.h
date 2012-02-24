#ifndef PLANE_H
#define PLANE_H

enum CollisionResult
{
	Bounce,
	Recycle,
	Stick
};

enum Collision
{
	Inside,
	Outside,
	OnPlane
};

struct Plane
{
	glm::vec3 normal;           // The plane's normal
	glm::vec3 point;            // A coplanar point within the plane
	float bounce_factor;     // Coefficient of restitution (or how bouncy the plane is)
	CollisionResult collision_result;  // What will particles do when they strike the plane

	Plane(glm::vec3 normal, glm::vec3 point, float bounce_factor = 0.5, CollisionResult collision_result = CollisionResult::Bounce)
	{
		this->normal = glm::normalize(normal);
		this->point = point;
		this->bounce_factor = bounce_factor;
		this->collision_result = collision_result;
	}

	/// Classifies a point against the plane passed
	Collision classify_point(glm::vec3 x, float epsilon = 0.01f)
	{
		float result = glm::dot((x - point), normal);
		if(result < -epsilon)
			return Collision::Outside;
		if(result > epsilon)
			return Collision::Inside;
		return Collision::OnPlane;
	}

	void collide(Particle &particle)
	{
		if (classify_point(particle.position) == Collision::Outside)
		{
			if (collision_result == CollisionResult::Bounce)
			{
				auto v = particle.velocity;
				//float cos_theta = glm::dot(-normal, v) / glm::length(normal) / glm::length(v);
				//auto v_normal = cos_theta * v;
				auto v_normal = glm::dot(-normal, v) * (-normal);
				auto v_tangential = v - v_normal;
				particle.velocity = v_tangential -bounce_factor * v_normal;
				auto d = particle.position - point;
				if (glm::dot(d, normal) < 0)
				{
					particle.position += glm::dot(-normal, d) * normal;
				}
			}
			else
			{
				if (collision_result == CollisionResult::Stick)
				{
					particle.velocity = glm::vec3(0, 0, 0);
				} 
				else
				{
					particle.alive = false;
				}
			}
		}
	}

	void collide_at_position(RigidBody &rb, glm::vec3 edge_position)
	{
		if (classify_point(edge_position) == Collision::Outside)
		{
			if (collision_result == CollisionResult::Bounce)
			{
				auto v = rb.velocity;
				//float cos_theta = glm::dot(-normal, v) / glm::length(normal) / glm::length(v);
				//auto v_normal = cos_theta * v;
				auto v_normal = glm::dot(-normal, v) * (-normal);
				auto v_tangential = v - v_normal;
				rb.velocity = v_tangential -bounce_factor * v_normal;
				auto d = edge_position - point;
				if (glm::dot(d, normal) < 0)
				{
					rb.position += glm::dot(-normal, d) * normal;
				}
			}
			else
			{
				if (collision_result == CollisionResult::Stick)
				{
					rb.velocity = glm::vec3(0, 0, 0);
					auto d = edge_position - point;
					if (glm::dot(d, normal) < 0)
					{
						rb.position += glm::dot(-normal, d) * normal;
					}
				}
			}
		}
	}

	void collide(RigidBody &rb)
	{
		auto edge_position = rb.position - normal * rb.geometry->radius();
		collide_at_position(rb, edge_position);
	}
};

#endif // PLANE_H
