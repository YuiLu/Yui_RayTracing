#pragma once
#include <cmath>
#include <cstdlib>
#include <limits>
#include <memory>
#include "ray.h"

using std::shared_ptr;
using std::make_shared;
using std::sqrt;

const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

inline double degrees_to_radians(double degrees) { return degrees * pi / 180; }
inline double ffmin(double a, double b) { return a <= b ? a : b; }
inline double ffmax(double a, double b) { return a >= b ? a : b; }

// [0, 1) 随机数
inline double random_double()
{
	return rand() / (RAND_MAX + 1.0);
}
// [min, max) 随机数
inline double random_double(double min, double max)
{
	return min + (max - min) * random_double();
}
inline double random_int(int min, int max)
{
	return static_cast<int>(random_double(min, max + 1));
}
// 钳制
inline double clamp(double x, double min, double max)
{
	if (x < min) return min;
	if (x > max) return max;
	return x;
}

// 极坐标实现随机单位向量
inline vec3 random_unit_vector()
{
	auto theta = random_double(0, 2 * pi); 
	auto phi = random_double(0, pi);
	return vec3(cos(theta) * sin(phi), sin(theta) * sin(phi), cos(phi));
}

//inline vec3 random_unit_vector()
//{
//	auto a = random_double(0, 2 * pi);
//	auto z = random_double(-1, 1);
//	auto r = sqrt(1 - z * z);
//	return vec3(r * cos(a), r * sin(a), z);
//}

//inline vec3 random_unit_vector()
//{
//	while (true)
//	{
//		auto p = vec3(random_double(-1,1), random_double(-1, 1), random_double(-1, 1));
//		if (p.length_squared() >= 1) continue;
//		return normalize(p);
//	}
//}

// 圆盘随机
inline vec3 random_in_unit_disk()
{
	while (true)
	{
		auto p = vec3(random_double(-1, 1), random_double(-1, 1), 0);
		if (p.length_squared() >= 1) continue;
		return p;
	}
}

// 线性插值
template<typename T>
inline T lerp(T A, T B, float alpha)
{
	return (1 - alpha) * A + alpha * B;
}

// Perlin插值
inline double perlin_lerp(double x)
{
	return 6 * pow(x, 5) - 15 * pow(x, 4) + 10 * pow(x, 3);
}

