#pragma once
#include "global.h"

// 关于柏林噪声：
// https://zhuanlan.zhihu.com/p/52054806

class perlin
{
public:
	perlin()
	{
		//// 随机数做哈希值
		//random_float = new double[point_count];

		// 偏移向量（梯度）做哈希值
		random_vector = new vec3[point_count];
		for (int i = 0; i < point_count; i++)
		{
			//random_float[i] = random_double();
			random_vector[i] = normalize(point(random_double(-1, 1), random_double(-1, 1), random_double(-1, 1)));
		}
		// 键值索引，三个维度分别来一次随机
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

	//// 引入插值前的噪声生成函数
	//double noise(const point& p) const
	//{
	//	// 按位与&是clamp，防止索引溢出，坐标前面乘的是重复度，值越大重复度越高
	//	auto i = static_cast<int>(4 * p.x()) & 255;
	//	auto j = static_cast<int>(4 * p.y()) & 255;
	//	auto k = static_cast<int>(4 * p.z()) & 255;
	//	// 三个维度按世界位置取到对应索引值之后，再做按位异或来合并哈希
	//	// XOR运算对于均匀分布的随机哈希表是一个非常好的合并方法
	//	return random_float[perm_x[i] ^ perm_y[j] ^ perm_z[k]];
	//}

	double noise(const point& p) const
	{
		// 世界坐标取小数
		auto u = p.x() - floor(p.x());
		auto v = p.y() - floor(p.y());
		auto w = p.z() - floor(p.z());
		//u = perlin_lerp(u);
		//v = perlin_lerp(v);
		//w = perlin_lerp(w);

		// 世界坐标取整
		auto i = static_cast<int>(floor(p.x()));
		auto j = static_cast<int>(floor(p.y()));
		auto k = static_cast<int>(floor(p.z()));
		// 三维数组代表三维空间内一个体素晶格的八个顶点
		vec3 c[2][2][2];

		// 三个维度按世界位置取到对应索引值之后，在组合后的哈希表中找到对应的哈希值
		// 经过三重嵌套循环，世界空间内整数坐标点就都对应了一个随机单位向量
		for (int di = 0; di < 2; di++)
			for (int dj = 0; dj < 2; dj++)
				for (int dk = 0; dk < 2; dk++)
					c[di][dj][dk] = random_vector[ perm_x[(i + di) & 255] ^ 
												   perm_y[(j + dj) & 255] ^ 
												   perm_z[(k + dk) & 255] ];
		// 然后再对这些格点上的随机值做三线性插值
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
	// 哈希函数的一部分（随机数法）
	static int* perlin_generate_perm()
	{
		// 初始化序列为一个递增整型序列
		auto p = new int[point_count];
		for (int i = 0; i < point_count; i++)
			p[i] = i;
		// 打乱
		permute(p, point_count);
		return p;
	}
	// 随机化
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

	//// 三线性插值
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

	// 柏林三线性插值
	static double perlin_interpolate(vec3 c[2][2][2], double u, double v, double w)
	{
		// 这里尝试了柏林噪声标准的插值函数，原版是Hermite cubic插值
		auto uu = perlin_lerp(u);
		auto vv = perlin_lerp(v);
		auto ww = perlin_lerp(w);
		auto result = 0.0;
		for (int i = 0; i < 2; i++)
			for (int j = 0; j < 2; j++)
				for (int k = 0; k < 2; k++) {
					// 晶格格点指向晶格内任意一点的权重向量
					vec3 weight_v = point(u, v, w) - point(i, j, k);
					result += (i * uu + (1 - i) * (1 - uu)) *
							  (j * vv + (1 - j) * (1 - vv)) *
							  (k * ww + (1 - k) * (1 - ww)) * 
							  dot(c[i][j][k], weight_v);
				}

		return result;
	}
};