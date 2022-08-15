#pragma once
#include "hittable.h"
#include <memory>  // 智能指针头文件
#include <vector>  // vector容器头文件

// 告诉编译器 shared_ptr 和 make_shared 是来自std库的，后面写的时候可以忽略std
using std::shared_ptr;
using std::make_shared;

class hittable_list : public hittable
{
public:
	hittable_list() {};
	/*object是hittable类的智能指针，hittable是虚基类，会在
      hittable_list调用构造函数时，根据传入的object类型（物
	  体类型），确定容器的类型*/
	hittable_list(shared_ptr<hittable> object) { add(object); }

	void clear() { objects.clear(); }
	void add(shared_ptr<hittable> object) { objects.push_back(object); }

	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;
	virtual bool bounding_box(aabb& output_box) const override;

public:
	std::vector<shared_ptr<hittable>> objects;
};

// 重写纯虚函数
bool hittable_list::hit(const ray& r, double t_min, double t_max, hit_record& rec) const
{
	hit_record temp_rec;
	bool hit_anything = false;
	// 通过记录临时值实现遮挡剔除
	auto closest_so_far = t_max;
	// 遍历容器
	for (const auto& object : objects)
	{
		if (object->hit(r, t_min, closest_so_far, temp_rec))
		{
			hit_anything = true;
			closest_so_far = temp_rec.t;
			rec = temp_rec;
		}
	}

	return hit_anything;
}

// 构建BVH包围盒
// 由于并非所有物体都能产生包围盒（比如无边界plane），所以这里函数返回bool
bool hittable_list::bounding_box(aabb& output_box) const
{
	if (objects.empty())
		return false;

	aabb temp_box;
	bool first_box = true;

	for (const auto& object : objects)
	{
		if (!object->bounding_box(temp_box)) return false;
		output_box = first_box ? temp_box : merge_box(output_box, temp_box);
		first_box = false;
	}

	return true;
}