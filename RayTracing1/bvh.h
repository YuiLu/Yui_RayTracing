#pragma once
#include "hittable_list.h"
#include <algorithm>

class bvh_node : public hittable
{
public:
	// 用构造函数（类外实现）初始化构造函数
	bvh_node() {}
	bvh_node(const hittable_list& list) : bvh_node(list.objects, 0, list.objects.size()) {}
	bvh_node(const std::vector<shared_ptr<hittable>>& src_objects, size_t start, size_t end);

	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;
	virtual bool bounding_box(aabb& output_box) const override;

public:
	// 左右孩子指针，可能指向bvh_node，也可能指向sphere等其他子类
	shared_ptr<hittable> left;
	shared_ptr<hittable> right;
	aabb box;
};

bool bvh_node::bounding_box(aabb& output_box) const
{
	output_box = box;
	return true;
}

// 检查当前节点的box是否命中，若是，则检查该节点的左右孩子（可能是bvh_node或sphere）是否命中
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
	// box_a和box_b起临时传递作用
	aabb box_a;
	aabb box_b;
	// 如果加入了没有包围盒的hittable对象，则打印报错
	if (!a->bounding_box(box_a) || !b->bounding_box(box_b))
		std::cerr << "No bounding box in bvh_node constructor.\n";
	// 没有包围盒的对象，minimum和maximum都是(0,0,0)
	return box_a.min().e[axis] < box_b.min().e[axis];
}

// sort()所使用的，各个轴向上的比较函数
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


// 在构造函数中完成BVH的构建
bvh_node::bvh_node(const std::vector<shared_ptr<hittable>>& src_objects, size_t start, size_t end)
{
	// 随机选取一个轴来分割
	auto objects = src_objects;
	int axis = random_int(0, 2);
	auto comparator = (axis == 0) ? box_x_compare : ((axis == 1) ? box_y_compare : box_z_compare); //函数指针

	size_t objects_length = end - start;

	// 若物体数量为1，则左右节点都指向该物体
	if (objects_length == 1)
	{
		left = right = objects[start];
	}
	// 若物体数量为2，则左右节点分别指向物体
	else if (objects_length == 2)
	{
		// 坐标小的给左孩子，坐标大的给右孩子
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
		// 排序，前半部分放左子树，后半部分放右子树，并递归调用构造函数
		std::sort(objects.begin() + start, objects.begin() + end, comparator);
		auto mid = start + objects_length / 2;
		left = make_shared<bvh_node>(objects, start, mid);
		right = make_shared<bvh_node>(objects, mid, end);
	}

	// box_left和box_right起临时传递作用
	aabb box_left, box_right;
	// 如果加入了没有包围盒的hittable对象，则打印报错
	if (!left->bounding_box(box_left) || !right->bounding_box(box_right))
		std::cerr << "No bounding box in bvh_node constructor.\n";
	// 合并子树包围盒
	box = merge_box(box_left, box_right);
}
