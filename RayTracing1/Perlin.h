#pragma once
#include "global.h"

// ���ڰ���������
// https://zhuanlan.zhihu.com/p/52054806

class perlin
{
public:
	perlin()
	{
		//// ���������ϣֵ
		//random_float = new double[point_count];

		// ƫ���������ݶȣ�����ϣֵ
		random_vector = new vec3[point_count];
		for (int i = 0; i < point_count; i++)
		{
			//random_float[i] = random_double();
			random_vector[i] = normalize(point(random_double(-1, 1), random_double(-1, 1), random_double(-1, 1)));
		}
		// ��ֵ����������ά�ȷֱ���һ�����
		perm_x = perlin_generate_perm();
		perm_y = perlin_generate_perm();
		perm_z = perlin_generate_perm();
	}

	~perlin()
	{
		//delete[] random_float;
		delete[] random_vector;
		delete[] perm_x;
		delete[] perm_y;
		delete[] perm_z;
	}

	//// �����ֵǰ���������ɺ���
	//double noise(const point& p) const
	//{
	//	// ��λ��&��clamp����ֹ�������������ǰ��˵����ظ��ȣ�ֵԽ���ظ���Խ��
	//	auto i = static_cast<int>(4 * p.x()) & 255;
	//	auto j = static_cast<int>(4 * p.y()) & 255;
	//	auto k = static_cast<int>(4 * p.z()) & 255;
	//	// ����ά�Ȱ�����λ��ȡ����Ӧ����ֵ֮��������λ������ϲ���ϣ
	//	// XOR������ھ��ȷֲ��������ϣ����һ���ǳ��õĺϲ�����
	//	return random_float[perm_x[i] ^ perm_y[j] ^ perm_z[k]];
	//}

	double noise(const point& p) const
	{
		// ��������ȡС��
		auto u = p.x() - floor(p.x());
		auto v = p.y() - floor(p.y());
		auto w = p.z() - floor(p.z());
		//u = perlin_lerp(u);
		//v = perlin_lerp(v);
		//w = perlin_lerp(w);

		// ��������ȡ��
		auto i = static_cast<int>(floor(p.x()));
		auto j = static_cast<int>(floor(p.y()));
		auto k = static_cast<int>(floor(p.z()));
		// ��ά���������ά�ռ���һ�����ؾ���İ˸�����
		vec3 c[2][2][2];

		// ����ά�Ȱ�����λ��ȡ����Ӧ����ֵ֮������Ϻ�Ĺ�ϣ�����ҵ���Ӧ�Ĺ�ϣֵ
		// ��������Ƕ��ѭ��������ռ������������Ͷ���Ӧ��һ�������λ����
		for (int di = 0; di < 2; di++)
			for (int dj = 0; dj < 2; dj++)
				for (int dk = 0; dk < 2; dk++)
					c[di][dj][dk] = random_vector[ perm_x[(i + di) & 255] ^ 
												   perm_y[(j + dj) & 255] ^ 
												   perm_z[(k + dk) & 255] ];
		// Ȼ���ٶ���Щ����ϵ����ֵ�������Բ�ֵ
		return perlin_interpolate(c, u, v, w);
	}

	double turbulence(const vec3& p, int depth = 7) const
	{
		auto accum = 0.0;
		vec3 temp_p = p;
		auto weight = 1.0;
		for (int i = 0; i < depth; i++)
		{
			accum += weight * noise(temp_p);
			weight *= 0.5;
			temp_p *= 2;
		}
		return fabs(accum);
	}

private:
	static const int point_count = 256;
	//double* random_float;
	vec3* random_vector;
	int* perm_x;
	int* perm_y;
	int* perm_z;
	// ��ϣ������һ���֣����������
	static int* perlin_generate_perm()
	{
		// ��ʼ������Ϊһ��������������
		auto p = new int[point_count];
		for (int i = 0; i < point_count; i++)
			p[i] = i;
		// ����
		permute(p, point_count);
		return p;
	}
	// �����
	static void permute(int* p, int n)
	{
		for (int i = n - 1; i > 0; i--)
		{
			int target = random_int(0, i);
			// swap
			int tmp = p[i];
			p[i] = p[target];
			p[target] = tmp;
		}
	}

	//// �����Բ�ֵ
	//static double trillinear_interpolate(double c[2][2][2], double u, double v, double w)
	//{
	//	auto result = 0.0;
	//	for (int i = 0; i < 2; i++)
	//		for (int j = 0; j < 2; j++)
	//			for (int k = 0; k < 2; k++)
	//				result += (i * u + (1 - i) * (1 - u)) * 
	//						  (j * v + (1 - j) * (1 - v)) * 
	//						  (k * w + (1 - k) * (1 - w)) * c[i][j][k];
	//	return result;
	//}

	// ���������Բ�ֵ
	static double perlin_interpolate(vec3 c[2][2][2], double u, double v, double w)
	{
		// ���ﳢ���˰���������׼�Ĳ�ֵ������ԭ����Hermite cubic��ֵ
		auto uu = perlin_lerp(u);
		auto vv = perlin_lerp(v);
		auto ww = perlin_lerp(w);
		auto result = 0.0;
		for (int i = 0; i < 2; i++)
			for (int j = 0; j < 2; j++)
				for (int k = 0; k < 2; k++) {
					// ������ָ�򾧸�������һ���Ȩ������
					vec3 weight_v = point(u, v, w) - point(i, j, k);
					result += (i * uu + (1 - i) * (1 - uu)) *
							  (j * vv + (1 - j) * (1 - vv)) *
							  (k * ww + (1 - k) * (1 - ww)) * 
							  dot(c[i][j][k], weight_v);
				}

		return result;
	}
};