#include <iostream>
#include <cmath>
using namespace std;
#include "../glm/glm/ext.hpp"
#include "../realtime_physics/distance.h"

inline float get_angle(glm::vec3 v1, glm::vec3 v2)
{
	return glm::degrees(acos(glm::dot(v1, v2) / glm::length(v1) / glm::length(v2)));
}

/// test if p3 and p4 lay on opposite sides of the segment p2-p1
bool intersect(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4)
{
	auto segment1 = p2 - p1;
	auto side1 = p3 - p1;
	auto side2 = p4 - p1;
	auto v1 = glm::cross(segment1, side1);
	auto v2 = glm::cross(segment1, side2);
	auto theta = glm::dot(v1, v2) / glm::length(v1) / glm::length(v2);
	auto angle = glm::degrees(acos(theta));
	printf("angle=%g\n", angle);
	if (abs(angle) <= 0.01)
	{
		auto angle1 = get_angle(segment1, side1);
		auto angle2 = get_angle(segment1, side2);
		return abs(angle1) < 0.01 || abs(angle2) < 0.01;
	}else
	{
		return abs(angle - 180) < 0.01;
	}
}

void main()
{
	glm::vec3 p1(0,0,0);
	glm::vec3 p2(2,0,0);
	glm::vec3 p3(2,-1,0);
	glm::vec3 p4(2,1,0);

	//auto angle1 = intersect(p1, p2, p3, p4);
	//printf("%s\n", angle1?"true":"false");
	//auto angle2 = intersect(p3, p4, p1, p2);
	//printf("%s\n", angle2?"true":"false");

	auto d = point_to_segment_distance(p1, p2, glm::vec3(1,1,0));
	printf("distance=%g\n", d);

	auto t = closest_time(p1, p2, p3, p4);
	auto distance = closest_distance(p1, p2, p3, p4);
	printf("time=%g\tdistance=%g\n", t, distance);
}
