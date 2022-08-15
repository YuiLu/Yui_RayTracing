#pragma once
#include "hittable_list.h"
#include <algorithm>

class bvh_node : public hittable
{
public:
	// �ù��캯��������ʵ�֣���ʼ�����캯��
	bvh_node() {}
	bvh_node(const hittable_list& list) : bvh_node(list.objects, 0, list.objects.size()) {}
	bvh_node(const std::vector<shared_ptr<hittable>>& src_objects, size_t start, size_t end);

	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;
	virtual bool bounding_box(aabb& output_box) const override;

public:
	// ���Һ���ָ�룬����ָ��bvh_node��Ҳ����ָ��sphere����������
	shared_ptr<hittable> left;
	shared_ptr<hittable> right;
	aabb box;
};

bool bvh_node::bounding_box(aabb& output_box) const
{
	output_box = box;
	return true;
}

// ��鵱ǰ�ڵ��box�Ƿ����У����ǣ�����ýڵ�����Һ��ӣ�������bvh_node��sphere���Ƿ�����
bool bvh_node::hit(const ray& r, double t_min, double t_max, hit_record& rec) const
{
	if (!box.hit(r, t_min, t_max))
		return false;

	bool hit_left = left->hit(r, t_min, t_max, rec);
	bool hit_right = right->hit(r, t_min, hit_left ? rec.t : t_max, rec);
	return hit_left || hit_right;
}


inline bool box_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b, int axis)
{
	// box_a��box_b����ʱ��������
	aabb box_a;
	aabb box_b;
	// ���������û�а�Χ�е�hittable�������ӡ����
	if (!a->bounding_box(box_a) || !b->bounding_box(box_b))
		std::cerr << "No bounding box in bvh_node constructor.\n";
	// û�а�Χ�еĶ���minimum��maximum����(0,0,0)
	return box_a.min().e[axis] < box_b.min().e[axis];
}

// sort()��ʹ�õģ����������ϵıȽϺ���
bool box_x_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b)
{
	return box_compare(a, b, 0);
}
bool box_y_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b)
{
	return box_compare(a, b, 1);
}
bool box_z_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b)
{
	return box_compare(a, b, 2);
}


// �ڹ��캯�������BVH�Ĺ���
bvh_node::bvh_node(const std::vector<shared_ptr<hittable>>& src_objects, size_t start, size_t end)
{
	// ���ѡȡһ�������ָ�
	auto objects = src_objects;
	int axis = random_int(0, 2);
	auto comparator = (axis == 0) ? box_x_compare : ((axis == 1) ? box_y_compare : box_z_compare); //����ָ��

	size_t objects_length = end - start;

	// ����������Ϊ1�������ҽڵ㶼ָ�������
	if (objects_length == 1)
	{
		left = right = objects[start];
	}
	// ����������Ϊ2�������ҽڵ�ֱ�ָ������
	else if (objects_length == 2)
	{
		// ����С�ĸ����ӣ������ĸ��Һ���
		if (comparator(objects[start], objects[start + 1]))
		{
			left = objects[start];
			right = objects[start + 1];
		}
		else
		{
			left = objects[start + 1];
			right = objects[start];
		}
	}
	else
	{
		// ����ǰ�벿�ַ�����������벿�ַ������������ݹ���ù��캯��
		std::sort(objects.begin() + start, objects.begin() + end, comparator);
		auto mid = start + objects_length / 2;
		left = make_shared<bvh_node>(objects, start, mid);
		right = make_shared<bvh_node>(objects, mid, end);
	}

	// box_left��box_right����ʱ��������
	aabb box_left, box_right;
	// ���������û�а�Χ�е�hittable�������ӡ����
	if (!left->bounding_box(box_left) || !right->bounding_box(box_right))
		std::cerr << "No bounding box in bvh_node constructor.\n";
	// �ϲ�������Χ��
	box = merge_box(box_left, box_right);
}
