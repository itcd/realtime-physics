#ifndef PHYSICS_H
#define PHYSICS_H

bool show_bounding_shpere()
{
	return button_bounding_sphere;
}

bool show_bounding_AABB()
{
	return button_AABB;
}

bool enable_gravity()
{
	return button_gravity;
}

#include "random.h"
#include "particle.h"
#include "rigid_body.h"
#include "plane.h"
#include "GJK.h"
#include "response.h"

#endif // PHYSICS_H
