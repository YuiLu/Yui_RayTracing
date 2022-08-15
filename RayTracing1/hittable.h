#pragma once
#include "global.h"
#include "aabb.h"

class material;

struct hit_record
{
	vec3 p;			//����
	vec3 normal;	//����
	double t;		//��
	bool front_face;//�������ж�
	// ����ָ��
	/* ����������������ཻʱ��hit_record�ṹ���е�materialָ��ͻ�
	   ����ֵ�Ӷ�ָ��һ�������materialָ�룬����������material��
	   ������mian���������ʱ����Ѿ����ú��˵�*/
	shared_ptr<material> mat_ptr;
	// ��������
	double u;
	double v;

	inline void set_front_normal(const ray& r, const vec3& outward_normal)
	{
		front_face = dot(r.Get_Direction(), outward_normal) < 0;
		normal = front_face ? outward_normal : -outward_normal;
	}
};

class hittable
{
public:
	// ���麯��
	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const = 0;
	virtual bool bounding_box(aabb& output_box) const = 0;
};


// ----------------------------���ع���Ŀ����Eigen���滻���´���-----------------------------------

class translate : public hittable
{
public:
	translate(shared_ptr<hittable> p, const vec3& displacement) : ptr(p), offset(displacement) {}

	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;
	virtual bool bounding_box(aabb& output_box) const override;

public:
	shared_ptr<hittable> ptr;
	vec3 offset;
};

bool translate::hit(const ray& r, double t_min, double t_max, hit_record& rec) const
{
	ray move_r(r.Get_Origin() - offset, r.Get_Direction());
	if (!ptr->hit(move_r, t_min, t_max, rec))
		return false;
	rec.p += offset;
	rec.set_front_normal(move_r, rec.normal);
	return true;
}

bool translate::bounding_box(aabb& output_box) const
{
	if (!ptr->bounding_box(output_box))
		return false;
	output_box = aabb(output_box.min() + offset, output_box.max() + offset);
	return true;
}

// ---------------------------------------------------------------------------------------------

class rotate_y : public hittable
{
public:
	rotate_y(shared_ptr<hittable> p, double angle);

	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;

	virtual bool bounding_box(aabb& output_box) const override
	{
		output_box = bbox;
		return hasbox;
	}
public:
	shared_ptr<hittable> ptr;
	double sin_theta;
	double cos_theta;
	bool hasbox;
	aabb bbox;
};

rotate_y::rotate_y(shared_ptr<hittable> p, double angle) : ptr(p)
{
	auto radians = degrees_to_radians(angle);
	sin_theta = sin(radians);
	cos_theta = cos(radians);
	hasbox = ptr->bounding_box(bbox);

	point min( infinity,  infinity,  infinity);
	point max(-infinity, -infinity, -infinity);

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			for (int k = 0; k < 2; k++)
			{
				auto x = i * bbox.max().x() + (1 - i) * bbox.min().x();
				auto y = j * bbox.max().y() + (1 - j) * bbox.min().y();
				auto z = k * bbox.max().z() + (1 - k) * bbox.min().z();

				auto newx = cos_theta * x + sin_theta * z;
				auto newz = -sin_theta * x + cos_theta * z;

				vec3 temp(newx, y, newz);

				for (int c = 0; c < 3; c++)
				{
					min[c] = ffmin(min[c], temp[c]);
					max[c] = ffmax(max[c], temp[c]);
				}
			}
		}
	}
	bbox = aabb(min, max);
}

bool rotate_y::hit(const ray& r, double t_min, double t_max, hit_record& rec) const
{
	auto origin = r.Get_Origin();
	auto direction = r.Get_Direction();
	// ��ת�����൱�ڷ�����ת����
	origin[0] = cos_theta * r.Get_Origin()[0] - sin_theta * r.Get_Origin()[2];
	origin[2] = sin_theta * r.Get_Origin()[0] + cos_theta * r.Get_Origin()[2];
	direction[0] = cos_theta * r.Get_Direction()[0] - sin_theta * r.Get_Direction()[2];
	direction[2] = sin_theta * r.Get_Direction()[0] + cos_theta * r.Get_Direction()[2];
	ray rotated_r(origin, direction);

	if (!ptr->hit(rotated_r, t_min, t_max, rec))
		return false;

	auto p = rec.p;
	auto normal = rec.normal;

	p[0] = cos_theta * rec.normal[0] + sin_theta * rec.normal[2];
	p[2] = -sin_theta * rec.normal[0] + cos_theta * rec.normal[2];
	normal[0] = cos_theta * rec.normal[0] + sin_theta * rec.normal[2];
	normal[2] = -sin_theta * rec.normal[0] + cos_theta * rec.normal[2];

	rec.p = p;
	rec.set_front_normal(rotated_r, normal);
	return true;
}