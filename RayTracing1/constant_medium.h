#pragma once
#include "hittable.h"
#include "material.h"

class constant_medium : public hittable
{
public:
	constant_medium(shared_ptr<hittable> b, double d, shared_ptr<texture> a) :
		boundary(b), neg_inv_density(-1/d), phase_function(make_shared<isotropic>(a)){}
	constant_medium(shared_ptr<hittable> b, double d, color c) :
		boundary(b), neg_inv_density(-1/d), phase_function(make_shared<isotropic>(c)){}

	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;
	virtual bool bounding_box(aabb& output_box) const override
	{
		return boundary->bounding_box(output_box);
	}
public:
	shared_ptr<hittable> boundary;
	shared_ptr<material> phase_function;
	double neg_inv_density;
};

bool constant_medium::hit(const ray& r, double t_min, double t_max, hit_record& rec) const
{
	hit_record rec1, rec2;

	// -inf ~ inf 是因为有可能会在天空球一样的物体的内部
	if (!boundary->hit(r, -infinity, infinity, rec1))
		return false;
	// 确保 rec2 会在 rec1 的ray的前方
	if (!boundary->hit(r, rec1.t+0.0001, infinity, rec2))
		return false;
	// clamp t in (t_min, t_max)
	if (rec1.t < t_min)		rec1.t = t_min;
	if (rec2.t > t_max)		rec2.t = t_max;
	// 不明白为什么要加这个if，从结果看并没有区别
	if (rec1.t >= rec2.t)	return false;
	// 保证光线求交是单向的射线
	if (rec1.t < 0)			rec1.t = 0;

	// http://psgraphics.blogspot.com/2013/11/scattering-in-constant-medium.html
	// 公式推导得：t = - (1/C)*ln(1-p)
	const auto ray_length = r.Get_Direction().length();
	const auto distance_inside_boundary = (rec2.t - rec1.t) * ray_length;
	const auto hit_distance = neg_inv_density * log(random_double());
	// 散射出边界
	if (hit_distance > distance_inside_boundary)
		return false;
	rec.t = rec1.t + hit_distance / ray_length;
	rec.p = r.at(rec.t);
	// 法线和front face都用不着，给个初值完事
	rec.normal = vec3(0, 0, 0);
	rec.front_face = true;
	rec.mat_ptr = phase_function;

	return true;
}