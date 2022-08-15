#pragma once
#include "vec3.h"

class ray
{
public:
	ray(){}
	ray(const point& ori, const vec3& dir) : origin(ori), direction(dir) {};

	point Get_Origin() const { return origin; }
	vec3 Get_Direction() const { return direction; }
	point at(double t) const 
	{
		return origin + t * direction;
	}
public:
	point origin;
	vec3 direction;
};