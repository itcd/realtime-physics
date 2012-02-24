#ifndef DISTANCE_H
#define DISTANCE_H

#define EPLISON 1e-6f

/// http://stackoverflow.com/questions/849211/shortest-distance-between-a-point-and-a-line-segment
float point_to_segment_distance(glm::vec3 v, glm::vec3 w, glm::vec3 p)
{
	// Return minimum distance between line segment vw and point p
	float length = glm::length(w - v);  // i.e. |w-v|^2 -  avoid a sqrt
	//printf("v w length=%g\n", length);
	if (abs(length) < EPLISON)
		return glm::distance(p, v);   // v == w case
	// Consider the line extending the segment, parameterized as v + t (w - v).
	// We find projection of point p onto the line. 
	// It falls where t = [(p-v) . (w-v)] / |w-v|^2
	float t = glm::dot(p - v, w - v) / (length * length);
	//printf("t=%g\n", t);
	if (t < 0.0)
		return glm::distance(p, v);       // Beyond the 'v' end of the segment
	else
	{
		if (t > 1.0)
			return glm::distance(p, w);  // Beyond the 'w' end of the segment
		else
		{
			auto projection = v + t * (w - v);  // Projection falls on the segment
			return glm::distance(p, projection);
		}
	}
}

/**
* Closest Point of Approach (CPA)
* http://softsurfer.com/Archive/algorithm_0106/algorithm_0106.htm
*/
// cpa_time(): compute the time of CPA for two tracks
//    Input:  two tracks Tr1 and Tr2
//    Return: the time at which the two tracks are closest
inline float closest_time(glm::vec3 p0, glm::vec3 p1, glm::vec3 q0, glm::vec3 q1)
{
	// P(t) = P0 + tu and Q(t) = Q0 + tv
	auto u = p1 - p0;
	auto v = q1 - q0;
	auto w0 = p0 - q0;
	auto u_minus_v = u - v;
	auto length = glm::length(u_minus_v);
	// If |u-v|=0, then the two points are traveling in the same direction at the same speed, and will always remain the same distance apart, so one can use tc = 0.
	if (length < EPLISON)
	{
		return 0;
	}
	else
	{
		return glm::dot(-w0, u_minus_v) / (length * length);
	}
}

// cpa_distance(): compute the distance at CPA for two tracks
//    Input:  two tracks Tr1 and Tr2
//    Return: the distance for which the two tracks are closest
float closest_distance(glm::vec3 p0, glm::vec3 p1, glm::vec3 q0, glm::vec3 q1)
{
	auto time = closest_time(p0, p1, q0, q1);
	if (time <= 0)
		return glm::distance(p0, q0);
	else
	{
		if (time >= 1)
		{
			return glm::distance(p1, q1);
		}else
		{
			auto p = p0 + time * (p1 - p0);
			auto q = q0 + time * (q1 - q0);
			return glm::distance(p, q);
		}
	}
}

float point_to_triangle_distance(glm::vec3 &normal, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 x)
{
	auto ab = b - a, ac = c - a, ax = x - a;
	// vertex voronoi a
	if (glm::dot(ax, ab) <= 0 && glm::dot(ax, ac) <= 0)
	{
		normal = a - x;
		return glm::distance(a, x);
	}
	else
	{
		// vertex voronoi b
		auto ba = a - b, bc = c - b, bx = x - b;
		if (glm::dot(bx, ba) <= 0 && glm::dot(bx, bc) <= 0)
		{
			normal = b - x;
			return glm::distance(b, x);
		}
		else
		{
			// vertex voronoi c
			auto ca = a - c, cb = b - c, cx = x - c;
			if (glm::dot(cx, ca) <= 0 && glm::dot(cx, cb) <= 0)
			{
				normal = c - x;
				return glm::distance(c, x);
			}
			else
			{
				// edge voronoi ba
				if (glm::dot(glm::cross(glm::cross(bc, ba), ba), bx) >= 0
					&& glm::dot(ax, ab) >=0 && glm::dot(bx, ba) >= 0)
				{
					normal = glm::cross(glm::cross(ba, bx), ba);
					return point_to_segment_distance(b, a, x);
				}
				else
				{
					// edge voronoi ac
					if (glm::dot(glm::cross(glm::cross(ab, ac), ac), ax) >= 0
						&& glm::dot(cx, ca) >=0 && glm::dot(ax, ac) >= 0)
					{
						normal = glm::cross(glm::cross(ac, ax), ac);
						return point_to_segment_distance(a, c, x);
					}
					else
					{
						// edge voronoi cb
						if (glm::dot(glm::cross(glm::cross(ca, cb), cb), cx) >= 0
							&& glm::dot(bx, bc) >=0 && glm::dot(cx, cb) >= 0)
						{
							normal = glm::cross(glm::cross(cb, cx), cb);
							return point_to_segment_distance(c, b, x);
						} 
						else
						{
							// in the region of the face
							auto ba = point_to_segment_distance(b, a, x);
							auto ac = point_to_segment_distance(a, c, x);
							auto cb = point_to_segment_distance(c, b, x);
							auto ba_ac = ba < ac ? ba : ac;
							// to do: update normal before return
							return ba_ac < cb ? ba_ac : cb;
						}
					}
				}
			}
		}
	}
}

glm::vec3 get_plane_normal(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 point_on_desired_side)
{
	auto ab = b - a;
	auto ac = c - a;
	auto abc = glm::cross(ab, ac);
	auto angle = glm::dot(abc, point_on_desired_side - a);
	return angle < 0 ? (-abc) : abc;
}

float point_to_plane_distance(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 p)
{
	auto normal = glm::normalize(get_plane_normal(a, b, c, p));
	auto ap = p - a;
	return glm::dot(ap, normal);
}

#endif // DISTANCE_H
