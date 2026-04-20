//Vector Struction with normalization, magnitude, projection, and angle finding functions
//operators overidden to perform relevant Vector arithmatic.
//Trevor LeCrone 2017

#include "Vec.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

double Vec::Magnitude(void) {
    return sqrt((x * x) + (y * y) + (z * z));
}

Vec Vec::Normalize(void) {
    double mag = Magnitude();
    return Vec((x / mag), (y / mag), (z / mag));
}

double Vec::FindAngle(Vec v2) {
    double dot_p = (*this * v2);
    double mag = Magnitude() * v2.Magnitude();
    return acos((dot_p / mag));
}

Vec Vec::Project(Vec v2) {
    double dot_p = (*this * v2);
    double mag = v2.Magnitude();
    return v2 * (dot_p / mag);
}

Vec Vec::Reflect(Vec norm) {
    double dot_p = (*this * norm);
    Vec ad_norm (norm * (2 * dot_p));
    return ad_norm + (*this * -1);
}

void Vec::PrintMe() {
    printf("x: %f  y: %f  z: %f\n", x, y, z);
}

void Vec::Set(double x_, double y_, double z_) {
    x = x_;
    y = y_;
    z = z_;
}

void Vec::SetX(double x_) { x = x_; }

void Vec::SetY(double y_) { y = y_; }

void Vec::SetZ(double z_) { z = z_; }

double Vec::X(void) { return x; }

double Vec::Y(void) { return y; }

double Vec::Z(void) { return z; }

Vec operator+ (const Vec& v1, const Vec& v2)
{
    return Vec(
        (v1.x + v2.x),
        (v1.y + v2.y),
        (v1.z + v2.z));
}

Vec operator- (const Vec& v1, const Vec& v2)
{
    return Vec(
        (v1.x - v2.x),
        (v1.y - v2.y),
        (v1.z - v2.z));
}


Vec operator* (const Vec& v1, double f)
{
    return Vec((v1.x * f),
               (v1.y * f),
               (v1.z * f));
}

double operator* (const Vec& v1, const Vec& v2)
{
    return (v1.x*v2.x) + (v1.y*v2.y) + (v1.z*v2.z);
}


Vec operator% (const Vec& v1, const Vec& v2)
{
    double new_x = (v1.y * v2.z) - (v1.z * v2.y);
    double new_y = (v1.z * v2.x) - (v1.x * v2.z);
    double new_z = (v1.x * v2.y) - (v1.y * v2.x);

    return Vec(new_x, new_y, new_z);
}
