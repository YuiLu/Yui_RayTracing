#pragma once
#include "hittable.h"

class sphere : public hittable
{
public:
	sphere() {}
	sphere(point cen, double rad, shared_ptr<material> mat) : center(cen), radius(rad), mat_ptr(mat) {};
	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;
	virtual bool bounding_box(aabb& output_box) const override;

public:
	point center;
	double radius;
	shared_ptr<material> mat_ptr;

private:
	static void get_sphere_uv(const point& p, double& u, double& v)
	{
		/* y = −cos(θ)
		   x = −cos(ϕ)sin(θ)
		   z =  sin(ϕ)sin(θ)  */
		/* u = ϕ / 2π	ϕ∈[0,2π]		u是xz平面的旋转角
		   v = θ / π	θ∈[0,π ]		v是y轴 [-1,1] 角 
		  (1, 0, 0) => <0.50 0.50>		(-1, 0, 0) => <0.00 0.50>
		  (0, 1, 0) => <0.50 1.00>		( 0,-1, 0) => <0.50 0.00>
		  (0, 0, 1) => <0.25 0.50>		( 0, 0,-1) => <0.75 0.50> */
		auto theta = acos(-p.y());
		auto phi = atan2(-p.z(), p.x()) + pi;
		u = phi / (2 * pi);
		v = theta / pi;
	}
};

// 重写纯虚函数
bool sphere::hit(const ray& r, double t_min, double t_max, hit_record& rec) const
{
	vec3 oc = r.Get_Origin() - center;
	auto a = r.Get_Direction().length_squared();
	auto half_b = dot(oc, r.Get_Direction());
	auto c = oc.length_squared() - radius * radius;

	auto discriminant = half_b * half_b - a * c;
	if (discriminant < 0) return false;
	auto sqrt_discriminant = sqrt(discriminant);
	// 求根
	auto root = (-half_b - sqrt_discriminant) / a;
	// 若较小根不在范围内，则换成较大根；若较大根也不在，那就求交失败
	if (root < t_min || root > t_max)
	{
		root = (-half_b + sqrt_discriminant) / a;
		if (root < t_min || root > t_max)
			return false;
	}
	// 用结构体记录结果
	rec.t = root;
	rec.p = r.at(rec.t);
	// 几何求交阶段判断球内外
	vec3 outward_normal = (rec.p - center) / radius;
	rec.set_front_normal(r, outward_normal);
	get_sphere_uv(outward_normal, rec.u, rec.v);
	rec.mat_ptr = mat_ptr;

	return true;
}

bool sphere::bounding_box(aabb& output_box) const
{
	output_box = aabb(center - vec3(radius, radius, radius),
					  center + vec3(radius, radius, radius));
	return true;
}