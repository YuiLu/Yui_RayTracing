#pragma once
#include "global.h"

class aabb
{
public:
	aabb() {}
	aabb(const point& a, const point& b) { minimum = a; maximum = b; }
	point min() const { return minimum; }
	point max() const { return maximum; }

	inline bool hit(const ray& r, double tmin, double tmax) const
	{
		for (int a = 0; a < 3; a++)
		{
			auto invD = 1.0f / r.Get_Direction()[a];
			auto t0 = (minimum[a] - r.Get_Origin()[a]) * invD;
			auto t1 = (maximum[a] - r.Get_Origin()[a]) * invD;
			// invD为负，则光线从t1进入，从t0离开
			if (invD < 0.0f)
				std::swap(t0, t1);
			// 
			tmin = t0 > tmin ? t0 : tmin;
			tmax = t1 < tmax ? t1 : tmax;
			if (tmax <= tmin)
				return false;
		}
		return true;
	}

public:
	point minimum;
	point maximum;
};

// 合并包围盒
aabb merge_box(aabb box0, aabb box1)
{
	point small(ffmin(box0.min().x(), box1.min().x()),
				ffmin(box0.min().y(), box1.min().y()),
				ffmin(box0.min().z(), box1.min().z()));
	point big(ffmax(box0.max().x(), box1.max().x()),
			  ffmax(box0.max().y(), box1.max().y()),
			  ffmax(box0.max().z(), box1.max().z()));
	return aabb(small, big);
}