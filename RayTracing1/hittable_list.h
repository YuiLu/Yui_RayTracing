#pragma once
#include "hittable.h"
#include <memory>  // ����ָ��ͷ�ļ�
#include <vector>  // vector����ͷ�ļ�

// ���߱����� shared_ptr �� make_shared ������std��ģ�����д��ʱ����Ժ���std
using std::shared_ptr;
using std::make_shared;

class hittable_list : public hittable
{
public:
	hittable_list() {};
	/*object��hittable�������ָ�룬hittable������࣬����
      hittable_list���ù��캯��ʱ�����ݴ����object���ͣ���
	  �����ͣ���ȷ������������*/
	hittable_list(shared_ptr<hittable> object) { add(object); }

	void clear() { objects.clear(); }
	void add(shared_ptr<hittable> object) { objects.push_back(object); }

	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;
	virtual bool bounding_box(aabb& output_box) const override;

public:
	std::vector<shared_ptr<hittable>> objects;
};

// ��д���麯��
bool hittable_list::hit(const ray& r, double t_min, double t_max, hit_record& rec) const
{
	hit_record temp_rec;
	bool hit_anything = false;
	// ͨ����¼��ʱֵʵ���ڵ��޳�
	auto closest_so_far = t_max;
	// ��������
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

// ����BVH��Χ��
// ���ڲ����������嶼�ܲ�����Χ�У������ޱ߽�plane�����������ﺯ������bool
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