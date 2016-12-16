#pragma once
#ifndef CAMERAH
#define CAMERAH
#define _USE_MATH_DEFINES
#include "ray.h"


float randomfloat() {
	float Rmax = 1.0f / ((float)RAND_MAX + 1);
	return (float)rand() * Rmax;
}

vec3 random_in_unit_disk() {
	vec3 p;
	float Rmax = 1.0f / ((float)RAND_MAX + 1);

	do {

		p = 2.0f * vec3(randomfloat(), randomfloat(), 0) - vec3(1, 1, 0);
	} while (dot(p, p) >= 1.0f);
	return p;
}

class camera {
public:
	camera(vec3 lookfrom,vec3 lookat,vec3 vup, float vfov ,float aspect,float aperture,float focus_dist) {
		lens_radius = aperture / 2.0f;
		float theta = vfov*(float)M_PI / 180;
		float half_height = tan(theta / 2);
		float half_width = aspect * half_height;
		origin = lookfrom;
		w = unit_vector(lookfrom - lookat);
		u = unit_vector(cross(vup, w));
		v = cross(w, u);
		//lower_left_corner = vec3(-half_width, -half_height, -1.0f);
		lower_left_corner = origin - half_width*u*focus_dist - half_height*v*focus_dist - w*focus_dist;
		horizontal = 2.0f * half_width * u*focus_dist;
		vertical = 2.0f * half_height * v*focus_dist;
		
	}

	ray get_ray(float s, float t) { 
		vec3 rd = lens_radius *random_in_unit_disk();
		vec3 offset = u * rd.x() + v * rd.y();
		return ray(origin + offset, lower_left_corner + s * horizontal + t * vertical - origin - offset); }
private:
	vec3 lower_left_corner;
	vec3 horizontal;
	vec3 vertical;
	vec3 origin;
	vec3 u, v, w;
	float lens_radius;
};
#endif