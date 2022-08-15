#pragma once
#include "global.h"
#include "texture.h"

struct hit_record;

class material
{
public:
	virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const = 0;
	virtual color emitted(double u, double v, const point& p) const { return color(0, 0, 0); }
};


// 漫反射材质
class lambertian : public material
{
public:
	lambertian(const color& a) : albedo(make_shared<solid_color>(a)) {}
	lambertian(shared_ptr<texture> a) : albedo(a){}

	virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override
	{
		auto scatter_direction = rec.normal + random_unit_vector();
		// safe direction
		if (scatter_direction.near_zero())
			scatter_direction = rec.normal;

		scattered = ray(rec.p, scatter_direction);
		attenuation = albedo->value(rec.u, rec.v, rec.p);
		return true;
	}

public:
	shared_ptr<texture> albedo;
};


// 金属
class metal : public material
{
public:
	metal(const color& a, double g) : albedo(a), gloss(g < 1 ? g : 1) {}

	virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override
	{
		vec3 reflected = reflect(normalize(r_in.direction), rec.normal);
		scattered = ray(rec.p, reflected + gloss * random_unit_vector());
		attenuation = albedo;
		return (dot(scattered.Get_Direction(), rec.normal) > 0);
	}
public:
	color albedo;
	double gloss;
};


// 绝缘体
/* 折射率参考：
   真空		1.0
   空气		1.000277
   冰		1.31
   水		1.333333
   玻璃		1.5
   钻石		2.417	*/
class dielectric : public material
{
public:
	dielectric(double i) : ior(i) {};
	virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override
	{
		attenuation = vec3(1.0, 1.0, 1.0);
		double eta;
		eta = rec.front_face ? (1.0 / ior) : ior;
		vec3 normalize_incident_ray = normalize(r_in.Get_Direction());
		// 全反射 &. 菲涅尔（schlick近似）
		double cos_theta = ffmin(dot(-normalize_incident_ray, rec.normal), 1.0);
		double sin_theta = sqrt(1.0 - cos_theta * cos_theta);
		if (eta * sin_theta > 1 || fresnel(cos_theta, eta) > random_double())
		{
			vec3 reflected = reflect(normalize_incident_ray, rec.normal);
			scattered = ray(rec.p, reflected);
			return true;
		}
		vec3 refracted = refract(normalize_incident_ray, rec.normal, eta);
		scattered = ray(rec.p, refracted);
		return true;
	}

public:
	double ior;

private:
	static double fresnel(double cosine, double eta)
	{
		auto r0 = (1 - eta) / (1 + eta);
		r0 *= r0;
		return r0 + (1 - r0) * pow((1 - cosine), 5);
	}
};

class diffuse_light : public material
{
public:
	diffuse_light(shared_ptr<texture> a) :emit(a) {}
	diffuse_light(color col) :emit(make_shared<solid_color>(col)) {}

	virtual bool scatter(const ray& r, const hit_record& rec, color& attenuation, ray& scattered) const override
	{
		return false;
	}

	virtual color emitted(double u, double v, const point& p) const override
	{
		return emit->value(u, v, p);
	}

public:
	shared_ptr<texture> emit;
};

class isotropic : public material
{
public:
	isotropic(color c) : albedo(make_shared<solid_color>(c)){}
	isotropic(shared_ptr<texture> a) : albedo(a){}

	virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override
	{
		scattered = ray(rec.p, random_unit_vector());
		attenuation = albedo->value(rec.u, rec.v, rec.p);
		return true;
	}

public:
	shared_ptr<texture> albedo;
};