#pragma once
#include "OBJ_Loader.h"
#include "hittable.h"

class triangle : public hittable
{
public:
	triangle() {}
	triangle(point _p0, point _p1, point _p2, shared_ptr<material> mat) :
		p0(_p0), p1(_p1), p2(_p2), mat_ptr(mat) {}
	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;
	virtual bool bounding_box(aabb& output_box) const override;

public:
	point p0, p1, p2;
	shared_ptr<material> mat_ptr;
};

// Möller-Trumbore
bool triangle::hit(const ray& r, double t_min, double t_max, hit_record& rec) const
{
	auto e1 = p1 - p0;
	auto e2 = p2 - p0;
	auto s = r.Get_Origin() - p0;
	auto s1 = cross(r.Get_Direction(), e2);
	auto s2 = cross(s, e1);
	auto det = dot(s1, e1);
	auto inv_det = 1 / det;
	auto u = dot(s, s1);
	auto v = dot(r.Get_Direction(), s2);
	// 无解或平行
	if (det == 0 || det < 0)
		return false;
	// 重心坐标在 [0, 1] 区间内
	if (u<0 || u>det || v<0 || u + v>det)
		return false;

	auto t = dot(s2, e2) * inv_det;
	if (t<t_min || t>t_max)
		return false;
	rec.t = t;

	rec.p = r.at(rec.t);
	vec3 outward_normal = normalize(cross(e1, e2));
	rec.set_front_normal(r, outward_normal);
	rec.u = u * inv_det;
	rec.v = v * inv_det;
	rec.mat_ptr = mat_ptr;
	return true;
}

bool triangle::bounding_box(aabb& output_box) const
{
	point pmin(ffmin(ffmin(p0.x(), p1.x()), p2.x()),
			   ffmin(ffmin(p0.y(), p1.y()), p2.y()),
			   ffmin(ffmin(p0.z(), p1.z()), p2.z()));
	point pmax(ffmax(ffmax(p0.x(), p1.x()), p2.x()),
			   ffmax(ffmax(p0.y(), p1.y()), p2.y()),
			   ffmax(ffmax(p0.z(), p1.z()), p2.z()));
	output_box = aabb(pmin, pmax);
	return true;
}