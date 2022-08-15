#pragma once
#include <iostream>
#include "global.h"
#include "Perlin.h"
#include "stb_image_rtw.h"

class texture
{
public:
	virtual color value(double u, double v, const point& p) const = 0;
};

// 静态纯色纹理
class solid_color : public texture
{
public:
	solid_color() {}
	solid_color(color col) : color_value(col) {}
	solid_color(double r, double g, double b) : solid_color(color(r, g, b)) {}

	virtual color value(double u, double v, const point& p) const override { return color_value; }

private:
	color color_value;
};

// 棋盘格纹理
class checker_texture : public texture
{
public:
	checker_texture() {}
	// 静态纹理构造函数
	checker_texture(shared_ptr<texture> _odd, shared_ptr<texture> _even) : odd(_odd), even(_even) {};
	// 程序生成纹理构造函数
	checker_texture(color col1, color col2) : odd(make_shared<solid_color>(col1)), even(make_shared<solid_color>(col2)) {};

	virtual color value(double u, double v, const point& p) const override
	{
		auto sines = sin(10 * p.x()) * sin(10 * p.y()) * sin(10 * p.z());
		if (sines < 0)
			return odd->value(u, v, p);
		else
			return even->value(u, v, p);
	}
public:
	shared_ptr<texture> odd;
	shared_ptr<texture> even;
};

// Perlin噪声
class noise_texture : public texture
{
public:
	noise_texture() {}
	noise_texture(double sc) : scale(sc) {}

	virtual color value(double u, double v, const point& p) const override
	{
		return color(1, 1, 1) * 0.5 * (1 + sin(scale * p.z() + 10 * noise.turbulence(p)));
	}
public:
	perlin noise;
	double scale;
};

// 图片纹理
class image_texture : public texture
{
public:
	int channels = 3;
	image_texture() :data(nullptr), width(0), height(0) {}
	image_texture(const char* filename)
	{
		data = stbi_load(filename, &width, &height, &channels, channels);
		if (!data)
		{
			std::cerr<< "ERROR: Could not load texture image file '" << filename << "'.\n";
			width = height = 0;
		}
	}
	~image_texture() { delete data; }

	virtual color value(double u, double v, const vec3& p) const override
	{
		if (data == nullptr)
			return color(1, 0, 1);
		// 钳制uv，并翻转v
		u = clamp(u, 0.0, 1.0);
		v = 1 - clamp(v, 0.0, 1.0);

		auto i = static_cast<int>(u * width);
		auto j = static_cast<int>(v * height);

		if (i >= width) i = width - 1;
		if (j >= height) j = width - 1;

		const auto color_scale = 1.0 / 255.0;
		auto pixel = data + j * width * channels + i * channels;
		return color(color_scale * pixel[0], color_scale * pixel[1], color_scale * pixel[2]);
	}

private:
	unsigned char* data;
	int width, height;
};