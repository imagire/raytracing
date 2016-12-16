#pragma once
#ifndef TRIANGLEH
#define TRIANGLEH

#include "hitable.h"

class triangle :public hitable {
public:
	triangle() {}
	triangle(vec3 p0,vec3 p1, vec3 p2, Material *mat):point0(p0),point1(p1),point2(p2), mat(mat) {}
	virtual bool hit(const ray& r, float tmin, float tmax, hit_record& rec)const;
private:
	vec3 point0, point1, point2;
	Material *mat;
};
bool triangle::hit(const ray& r, float t_min, float t_max, hit_record& rec)const {
	vec3 t0 = point1 - point0;
	vec3 t1 = point2 - point0;
	vec3 t2 = point2 - point1;
	vec3 normal = unit_vector(cross(t1, t0));

	vec3 op = point0 - r.origin();
	float opn = dot(op, normal);
	float direcn = dot(r.direction(), normal);

	float t = opn / direcn;
	if ( t < 0) {
		return false;
	}

	vec3 p = r.origin() + t * r.direction();
	vec3 d = p - point0;
	float c = dot(cross(d, t0), normal);
	if (c<0) {
		return false;
	}
	d = p - point1;
	c = dot(cross(d, t2), normal);
	if (c<0) {
		return false;
	}
	d = p - point2;
	c = dot(cross(d, -t1), normal);
	if (c<0) {
		return false;
	}
	rec.p = r.point_at_parameter(rec.t);
	rec.normal = normal;
	rec.mat_ptr = mat;
	return true;
}
#endif // !TRIANGLEH
