#pragma once
#include "global.h"

class camera
{
public:
	camera(
		point camera_location, point look_at, vec3 up,
		double vfov, //垂直视场角，角度制
		double aspect_ratio, double aperture, double focus_dist
	)
	{
		auto vfov_rad = degrees_to_radians(vfov);
		auto h = tan(vfov_rad / 2);
		auto viewport_height = 2.0 * h;
		auto viewport_width = aspect_ratio * viewport_height;

		w = normalize(camera_location - look_at);
		u = normalize(cross(up, w));
		v = cross(w, u);

		origin = camera_location;
		horizontal = focus_dist * viewport_width * u;
		vertical = focus_dist * viewport_height * v;
		lower_left_corner = origin - horizontal / 2 - vertical / 2 - focus_dist * w;

		lens_radius = aperture / 2;
	}

	ray get_ray(double s, double t) const
	{
		vec3 rd = lens_radius * random_in_unit_disk();
		vec3 offset = u * rd.x() + v * rd.y();
		return ray(origin + offset, lower_left_corner + s * horizontal + t * vertical - origin - offset);
	}

private:
	point origin;
	point lower_left_corner;
	vec3 horizontal;
	vec3 vertical;
	vec3 u, v, w;
	double lens_radius;
};