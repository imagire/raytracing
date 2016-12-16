#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include "camera.h"
#include "sphere.h"
#include "triangle.h"
#include "hitable_list.h"


typedef struct _RGB {
	double r;    //red
	double g;    //green
	double b;    //blue
}RGB;

typedef struct _PPM
{
    RGB **pixels;   //pixels[width][height]
    int width;
    int height;
}PPM;




float schlick(float cos, float ref_idx) {
	float r0 = (1.0f - ref_idx) / (1.0f + ref_idx);
	r0 = r0*r0;
	return r0 + (1.0f - r0)*pow((1.0f - cos), 5);
}

class Material
{
public:
	virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scatterd) const = 0;
};

vec3 random_in_unit_sphere() {
	vec3 p;
	do {
		p = 2.0f*vec3(randomfloat(), randomfloat(), randomfloat()) - vec3(1.0f, 1.0f, 1.0f);
	} while (p.squared_length() >= 1.0f);
	return p;
}


class lambartian :public Material {
public:
	lambartian(const vec3& a) :albedo(a) {}
	virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scatterd)const {
		vec3 target = rec.p + rec.normal + random_in_unit_sphere();
		scatterd = ray(rec.p, target - rec.p);
		attenuation = albedo;
		return true;
	}
private:
	vec3 albedo;
};

class metal :public Material {
public:
	metal(const vec3& a, float f) :albedo(a) { if (f < 1.0f)fuzz = f; else fuzz = 1.0f; }
	virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scatterd)const {
		vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
		scatterd = ray(rec.p, reflected + fuzz*random_in_unit_sphere());
		attenuation = albedo;
		return (dot(scatterd.direction(), rec.normal) > 0);
	}
private:
	vec3 albedo;
	float fuzz;
};
class dielectic :public Material {
public:
	dielectic(float ri) : ref_idx(ri){}
	virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scatterd)const {
		vec3 outward_normal;
		vec3 reflected = reflect(r_in.direction(), rec.normal);
		float ni_over_nt;
		attenuation = vec3(1.0f, 1.0f, 1.0f);
		vec3 refracted;
		float refract_prob;
		float cos;
		if (dot(r_in.direction(), rec.normal) > 0.0f) {
			outward_normal = -rec.normal;
			ni_over_nt = ref_idx;
			cos = ref_idx * dot(r_in.direction(), rec.normal) / r_in.direction().length();
		}
		else
		{
			outward_normal = rec.normal;
			ni_over_nt = 1.0f / ref_idx;
			cos =  -dot(r_in.direction(), rec.normal) / r_in.direction().length();
		}
		if (refract(r_in.direction(), outward_normal, ni_over_nt, refracted)) {
			refract_prob = schlick(cos, ref_idx);
			
		}
		else
		{
			scatterd = ray(rec.p, reflected);
			refract_prob = 1.0f;
		}
		if (randomfloat() < refract_prob) {
			scatterd = ray(rec.p, reflected);
		}
		else
		{
			scatterd = ray(rec.p, refracted);
		}
		return true;
	}
private:
	float ref_idx;
};

vec3 color(const ray& r,hitable *world,int depth) {
	hit_record rec;
	if (world->hit(r, 0.0f, FLT_MAX, rec)) {
		ray scattered;
		vec3 attenuation;
		if (depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
			return attenuation*color(scattered, world, depth + 1);
		}
		else
		{
			return vec3(0, 0, 0);
		}
	}
	else {
		vec3 unit_direction = unit_vector(r.direction());
		float t = 0.5f*(unit_direction.y() + 1.0f);
		return (1.0f - t) * vec3(1.0f, 1.0f, 1.0f) + t*vec3(0.5f, 0.7f, 1.0f);
	}
	
}

void free_ppm(PPM *ppm)
{
	if (ppm->pixels) {
		if (ppm->pixels[0]) free(ppm->pixels[0]);
		free(ppm->pixels);
	}

	ppm->pixels = NULL;
	ppm->width = 0;
	ppm->height = 0;
}

void create_ppm(PPM *ppm, int width, int height)
{
	int i;
	int size = width*height;

	free_ppm(ppm);

	ppm->width = width;
	ppm->height = height;

	//配列の確保
	ppm->pixels = (RGB**)malloc(ppm->width * sizeof(RGB*));
	ppm->pixels[0] = (RGB*)malloc(size * sizeof(RGB));
	for (i = 1; i<ppm->width; i++) ppm->pixels[i] = ppm->pixels[i - 1] + ppm->height;
}

hitable *random_scene() {
	int n = 500;
	hitable **list = new hitable*[n + 1];
	list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambartian(vec3(0.5f, 0.5f, 0.5f)));
	int i = 1;
	for (int a = -11; a < 11; a++) {
		for (int b = -11; b < 11; b++) {
			float choose_mat = randomfloat();
			vec3 center(a + 0.9f*randomfloat(), 0.2f, b + 0.9f*randomfloat());
			if ((center - vec3(4, 0.2f, 0)).length() > 0.9f) {
				if (choose_mat < 0.8f) {  // diffuse
					list[i++] = new sphere(center, 0.2f, new lambartian(vec3(randomfloat()*randomfloat(), randomfloat()*randomfloat(), randomfloat()*randomfloat())));
				}
				else if (choose_mat < 0.95f) { // metal
					list[i++] = new sphere(center, 0.2f,
						new metal(vec3(0.5f*(1 + randomfloat()), 0.5f*(1 + randomfloat()), 0.5f*(1 + randomfloat())), 0.5f*randomfloat()));
				}
				else {  // glass
					list[i++] = new sphere(center, 0.2f, new dielectic(1.5f));
				}
			}
		}
	}

	list[i++] = new sphere(vec3(0, 1, 0), 1.0f, new dielectic(1.5f));
	list[i++] = new sphere(vec3(-4, 1, 0), 1.0f, new lambartian(vec3(0.4f, 0.2f, 0.1f)));
	list[i++] = new sphere(vec3(4, 1, 0), 1.0f, new metal(vec3(0.7f, 0.6f, 0.5f), 0.0f));

	return new hitable_list(list, i);
}

void save_to_file(char *file_name, PPM *ppm)
{
	int  x, y;
	FILE *fp = fopen(file_name, "w");

	//ファイルフォーマットの書き込み
	fprintf(fp, "P3\n");

	//コメントの書き込み
	fprintf(fp, "# CREATOR: k-n-gk\n");

	//画像サイズと最大値の書き込み
	fprintf(fp, "%d %d\n", ppm->width, ppm->height);
	fprintf(fp, "255\n");

	for (y = 0; y<ppm->height; y++) {
		for (x = 0; x<ppm->width; x++) {
			fprintf(fp, "%d ", (int)(255.99 * ppm->pixels[x][y].r));
			fprintf(fp, "%d ", (int)(255.99 * ppm->pixels[x][y].g));
			fprintf(fp, "%d\n", (int)(255.99 * ppm->pixels[x][y].b));
		}
	}

	fclose(fp);
}

int main() {
	
	PPM  pict;
	
	pict.pixels = NULL;
	pict.width =200;
	pict.height = 100;
	int nx = pict.width;
	int ny = pict.height;
	float invx = 1.0f / float(nx);
	float invy = 1.0f / float(ny);
	int ns = 100;
	vec3 lookfrom(0, 2, 8);
	vec3 lookat(0, 1, 0);
	float dist_to_focus = 10.0f;
	float aperture = 0.1f;
	camera cam(lookfrom, lookat, vec3(0, 1, 0), 20, (float)nx * invy, aperture, dist_to_focus);
	float R = cos((float)M_PI / 4.0f);
	hitable *list[4];
	list[0] = new sphere(vec3(0.0f, -100.5f, -1.0f), 100.0f, new metal(vec3(0.3f, 0.8f, 0.3f) ,0.5));
	list[1] = new triangle(vec3(-1.0f, 1.0f, -3.0f), vec3(0.0f, 2.0f, 0.0f), vec3(1.0f, 1.0f, -3.0f), new metal(vec3(0.8f, 0.5f, 0.0f),0.0f));
	list[2] = new sphere(vec3(-1.0f, 0.0f, -1.0f), 0.5f, new metal(vec3(0.8f, 0.5f, 0.0f),0.2f));
	list[3] = new sphere(vec3(1.0f, 0.0f, -1.0f), 0.5f, new metal(vec3(0.8f, 0.5f, 0.0f),0.2f));
	hitable *world = new hitable_list(list, 4);

	//world = random_scene();

	create_ppm(&pict, pict.width, pict.height);
	int y = 0;
	for (int j = ny - 1; j >= 0; j--) {
		for (int i = 0; i < nx; i++) {
			vec3 col(0, 0, 0);
			for (int s = 0; s < ns; s++) {
				float u = float(i + randomfloat()) * invx;
				float v = float(j + randomfloat()) * invy;
				ray r = cam.get_ray(u, v);
				//vec3 p = r.point_at_parameter(2.0f);
				col += color(r, world,0);
			}
			col /= float(ns);
			col = vec3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));
			pict.pixels[i][y].r = col[0];
			pict.pixels[i][y].g = col[1];
			pict.pixels[i][y].b = col[2];

		}
		y++;
	}
	save_to_file("test.ppm", &pict);
	free_ppm(&pict);
	return 0;
}
