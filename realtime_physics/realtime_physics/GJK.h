#ifndef GJK_H
#define GJK_H

#include <tuple>
#include "distance.h"
#include "simplex.h"

glm::vec3 get_farthest_point_in_direction(glm::vec3 direction, const std::vector<glm::vec3> &vertices)
{
	auto d = glm::normalize(direction);
	float max = glm::dot(d, vertices[0]);
	auto farthest_point = vertices[0];

	for (auto i=vertices.begin(); i!=vertices.end(); i++)
	{
		auto projection = glm::dot(d, *i);
		if (projection > max)
		{
			max = projection;
			farthest_point = *i;
		}
	}

	return farthest_point;
}

#include "response.h"

glm::vec3 support(glm::vec3 d, const std::vector<glm::vec3> &vertices1, const std::vector<glm::vec3> &vertices2)
{
	// d is a vector direction (doesn't have to be normalized)
	// get points on the edge of the shapes in opposite directions
	auto p1 = get_farthest_point_in_direction(d, vertices1);
	auto p2 = get_farthest_point_in_direction(-d, vertices2);

	// perform the Minkowski Difference
	auto p3 = p1 - p2;

	// p3 is now a point in Minkowski space on the edge of the Minkowski Difference
	return p3;

}

std::tuple<glm::vec3, glm::vec3, glm::vec3> support_tuple(glm::vec3 d, const std::vector<glm::vec3> &vertices1, const std::vector<glm::vec3> &vertices2)
{
	// d is a vector direction (doesn't have to be normalized)
	// get points on the edge of the shapes in opposite directions
	auto p1 = get_farthest_point_in_direction(d, vertices1);
	auto p2 = get_farthest_point_in_direction(-d, vertices2);

	// perform the Minkowski Difference
	auto p3 = p1 - p2;

	// p3 is now a point in Minkowski space on the edge of the Minkowski Difference
	std::tuple<glm::vec3, glm::vec3, glm::vec3> t(p3, p1, p2);
	return t;
}

glm::vec3 triple_product(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3)
{
	return glm::cross(glm::cross(v1, v2), v3);
}

bool same_side(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 p1, glm::vec3 p2)
{
	auto v1v2 = v2 - v1;
	auto v1v3 = v3 - v1;
	auto n = glm::cross(v1v2, v1v3);
	auto v1p1 = p1 - v1;
	auto v1p2 = p2 - v1;
	auto r1 = glm::dot(n, v1p1);
	auto r2 = glm::dot(n, v1p2);
	return r1 * r2 >= 0;
}

bool point_in_tetrahedron(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 v4, glm::vec3 p1)
{
	return same_side(v1, v2, v3, v4, p1)
		&& same_side(v1, v2, v4, v3, p1)
		&& same_side(v1, v3, v4, v2, p1)
		&& same_side(v2, v3, v4, v1, p1);
}

glm::vec3 get_perpendicular_direction(Simplex<glm::vec3> s)
{
	auto v1 = s.getA();
	auto v2 = s.getB();
	auto v3 = s.getC();
	auto v1v2 = v2 - v1;
	auto v1v3 = v3 - v1;
	auto n = glm::cross(v1v2, v1v3);
	auto r = glm::dot(n, -v1);
	return r < 0 ? -n : n;
}

glm::vec3 find_normal(Simplex<glm::vec3> simplex, const RigidBody &A, const RigidBody &B, const std::vector<glm::vec3> &vertices1, const std::vector<glm::vec3> &vertices2)
{
	auto ab = B.position - A.position;
	auto ba = A.position - B.position;
	auto p_a = get_farthest_point_in_direction(ab, A.geometry->vertices(A.orientation, A.position));
	auto p_b = get_farthest_point_in_direction(ba, B.geometry->vertices(B.orientation, B.position));
	auto normal = glm::normalize(ba);
	auto d_a = glm::dot(p_a - B.position, normal);
	auto d_b = glm::dot(p_b - B.position, normal);

	//// for test
	//if (d_a - d_b < EPLISON)
	//{
	//	//auto diff = abs(d_b - d_a) + 2 * EPLISON;
	//	printf("overlap in center to center direction\n");
	//}

	auto dist = abs(d_b - d_a);
	std::vector<float> distances;
	std::vector<glm::vec3> directions;
	distances.push_back(dist);
	directions.push_back(normal);

	auto a = simplex.getA();
	auto b = simplex.getB();
	auto c = simplex.getC();
	auto d = simplex.getD();
	directions.push_back(get_plane_normal(a, b, c, d));
	directions.push_back(get_plane_normal(a, b, d, c));
	directions.push_back(get_plane_normal(a, c, d, b));
	directions.push_back(get_plane_normal(b, c, d, a));
	distances.push_back(point_to_plane_distance(a, b, c, d));
	distances.push_back(point_to_plane_distance(a, b, d, c));
	distances.push_back(point_to_plane_distance(a, c, d, b));
	distances.push_back(point_to_plane_distance(b, c, d, a));

	//// for test
	//auto origin = glm::vec3(0, 0, 0);
	//if (point_in_tetrahedron(a, b, c, d, origin))
	//{
	//	printf("GJK: the simplex contains origin\n");
	//}
	//else
	//{
	//	printf("GJK: the simplex doesn't contain origin\n");
	//}

	auto min = distances[0];
	auto min_index = 0;
	for (int i=0; i<distances.size(); i++)
	{
		if (distances[i] < min)
		{
			min = distances[i];
			min_index = i;
		}
	}

	auto result_normal = distances[min_index] * glm::normalize(directions[min_index]);

	//point_to_triangle_distance(normal, a, b, c, origin);
	return result_normal;
}

// 3D GJK implemetation in XNA / c#
// https://mollyrocket.com/forums/viewtopic.php?p=7015
bool update_simplex_and_direction(Simplex<glm::vec3> &simplex, glm::vec3 &direction)
{
	// if it's the line segment case
	if (simplex.points.size() == 2)
	{
		// A is the point added last to the simplex
		auto a = simplex.getA();
		auto ao = -a;
		auto b = simplex.getB();
		auto ab = b - a;
		if (glm::dot(ab, ao) > 0)
		{
			direction = triple_product(ab, ao, ab);
		}
		else
		{
			direction = ao;
		}
	}
	else
	{
		// if it's the triangle case
		if (simplex.points.size() == 3)
		{
			// A is the point added last to the simplex
			auto a = simplex.getA();
			auto ao = -a;
			auto b = simplex.getB();
			auto ab = b - a;
			auto c = simplex.getC();
			auto ac = c - a;
			auto abc = glm::cross(ab, ac);

			if (glm::dot(glm::cross(abc, ac), ao) > 0)
			{
				if (glm::dot(ac, ao) > 0)
				{
					simplex.clear();
					simplex.add(c);
					simplex.add(a);
					direction = triple_product(ac, ao, ac);
				}
				else
				{
					if (glm::dot(ab, ao) > 0)
					{
						simplex.clear();
						simplex.add(b);
						simplex.add(a);
						direction = triple_product(ab, ao, ab);
					}
					else
					{
						simplex.clear();
						simplex.add(a);
						direction = ao;
					}
				}
			}
			else
			{
				if (glm::dot(glm::cross(ab, abc), ao) > 0)
				{
					if (glm::dot(ab, ao) > 0)
					{
						simplex.clear();
						simplex.add(b);
						simplex.add(a);
						direction = triple_product(ab, ao, ab);
					}
					else
					{
						simplex.clear();
						simplex.add(a);
						direction = ao;
					}
				} 
				else
				{
					if (glm::dot(abc, ao) > 0)
					{
						// the simplex stays A, B, C 
						direction = abc; 
					}
					else
					{
						simplex.clear();
						simplex.add(b);
						simplex.add(c);
						simplex.add(a);
						direction = -abc;
					}
				}
			}
		}
		else
		{
			// if it's the tetrahedron case
			if (simplex.points.size() == 4)
			{
				auto a = simplex.getA();
				auto b = simplex.getB();
				auto c = simplex.getC();
				auto d = simplex.getD();
				auto ao = -a;
				auto ab = b - a;
				auto ac = c - a;
				auto ad = d - a;
				auto abc = glm::cross(ab, ac);
				auto acd = glm::cross(ac, ad);
				auto adb = glm::cross(ad, ab);

				// the side (positive or negative) of B, C and D relative to the planes of ACD, ADB and ABC respectively
				auto BsideOnACD = glm::dot(acd, ab);// Math.Sign(ACD.Dot(AB));
				auto CsideOnADB = glm::dot(adb, ac);//Math.Sign(ADB.Dot(AC));
				auto DsideOnABC = glm::dot(abc, ad);//Math.Sign(ABC.Dot(AD));

				// whether the origin is on the same side of ACD/ADB/ABC as B, C and D respectively
				bool ABsameAsOrigin = glm::dot(acd, ao) * BsideOnACD > 0;
				bool ACsameAsOrigin = glm::dot(adb, ao) * CsideOnADB > 0;
				bool ADsameAsOrigin = glm::dot(abc, ao) * DsideOnABC > 0;

				// if the origin is on the same side as all B, C and D, the origin is inside the tetrahedron and thus there is a collision
				if (ABsameAsOrigin && ACsameAsOrigin && ADsameAsOrigin)
				{
					return true;
				}
				// if the origin is not on the side of B relative to ACD 
				else if (!ABsameAsOrigin)
				{ 
					//B is farthest from the origin among all of the tetrahedron's points, so remove it from the list and go on with the triangle case 
					simplex.remove(b); 
					//the new direction is on the other side of ACD, relative to B
					direction = BsideOnACD >= 0 ? (-acd) : acd;
				}
				// if the origin is not on the side of C relative to ADB 
				else if (!ACsameAsOrigin)
				{
					//C is farthest from the origin among all of the tetrahedron's points, so remove it from the list and go on with the triangle case
					simplex.remove(c); 
					//the new direction is on the other side of ADB, relative to C 
					direction = CsideOnADB >= 0 ? (-adb) : adb;
				}
				// if the origin is not on the side of D relative to ABC 
				else //if (!ADsameAsOrigin)
				{
					//D is farthest from the origin among all of the tetrahedron's points, so remove it from the list and go on with the triangle case
					simplex.remove(d);
					//the new direction is on the other side of ABC, relative to D 
					direction = DsideOnABC >= 0 ? (-abc) : abc;
				}

				// go on with the triangle case 
				// TODO: maybe we should restrict the depth of the recursion, just like we restricted the number of iterations in BodiesIntersect?
				return update_simplex_and_direction(simplex, direction); 
			}
		}
	}
	// no intersection found on this iteration 
	return false;
}

bool GJK_intersect(RigidBody A, RigidBody B, glm::vec3 &normal)
{
	auto vertices1 = A.geometry->vertices(A.orientation, A.position);
	auto vertices2 = B.geometry->vertices(B.orientation, B.position);
	auto direction = A.position - B.position;

	// get the first Minkowski Difference point
	auto p1 = support(direction, vertices1, vertices2);
	Simplex<glm::vec3> simplex;
	simplex.add(p1);
	direction = -direction;

	while (true)
	{
		// add a new point to the simplex
		auto p = support(direction, vertices1, vertices2);
		//printf("add point %g,%g,%g\n", p.x, p.y, p.z);
		simplex.add(p);

		// make sure that the last point we added actually passed the origin
		if (glm::dot(simplex.getA(), direction) <= 0)
		{
			return false;
		}
		else
		{
			// if  the origin is in the current simplex then there is a collision
			if (update_simplex_and_direction(simplex, direction))
			{
				normal = find_normal(simplex, A, B, vertices1, vertices2);
				return true;
			}
		}
	}
}

void GJK_collide(RigidBody &rb0, RigidBody &rb)
{
	auto d = closest_distance(rb0.position_old, rb0.position, rb.position_old, rb.position);
	if (d <= rb0.geometry->radius() + rb.geometry->radius())
	{
		glm::vec3 normal;
		if (GJK_intersect(rb0, rb, normal))
		{
			auto a = rb0.position, b = rb.position;
			//rb0.respond(rb);
			rb_rb_response(rb0, rb, normal);
		}
	}
}

#endif // GJK_H
