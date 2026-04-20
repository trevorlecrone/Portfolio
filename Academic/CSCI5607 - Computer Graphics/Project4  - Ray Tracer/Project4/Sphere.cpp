#include "Sphere.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

double Sphere::CheckCollision(Ray ray) {
    //a = 1 since r we normalized our ray
    Vec norm_r = ray.GetDir();
    Vec point = ray.GetOrigin();
    //b
    double b = 2 * ((point - center) * norm_r);
    //printf("(norm_r * 2): %f,%f,%f\n", (norm_r * 2).x, (norm_r * 2).y, (norm_r * 2).z);
    //printf("(center - point): %f,%f,%f\n", (center - point).x, (center - point).y, (center - point).z);
    //printf("b: %f\n\n", b);
    //c
    double c = (((point - center) * (point - center)) - (radius * radius));
    //printf("c: %f\n\n", c);
    return quadratic(1.0, b, c);
}
bool Sphere::CheckShadow(Ray ray, double offset, double max) {
    double t = CheckCollision(ray);
    if (max == -999.0) {
        if (t < offset) {
            return false;
        }
        else {
            return true;
        }
    }
    else {
        if (t < offset || t > max) {
            return false;
        }
        else {
            return true;
        }
    }
}

Vec Sphere::GetNormal(Vec point) {
    return point - center;
}

double Sphere::GetRadius() {
    return radius;
}