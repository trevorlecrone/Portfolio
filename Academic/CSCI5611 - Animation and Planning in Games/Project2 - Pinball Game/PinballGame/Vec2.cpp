#include "Vec2.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

double Vec2::Magnitude(void) {
    return sqrt((x * x) + (y * y));
}

Vec2 Vec2::Normalize(void) {
    double mag = Magnitude();
    return Vec2((x / mag), (y / mag));
}

double Vec2::FindAngle(Vec2 v2) {
    double dot_p = (*this * v2);
    double mag = Magnitude() * v2.Magnitude();
    return acos((dot_p / mag));
}

Vec2 Vec2::Project(Vec2 v2) {
    double dot_p = (*this * v2);
    double mag = v2.Magnitude();
    return v2 * (dot_p / mag);
}

Vec2 Vec2::Reflect(Vec2 norm) {
    double dot_p = (*this * norm);
    return *this - (norm * (2 * dot_p));
}

void Vec2::PrintMe() {
    printf("x: %f  y: %f\n", x, y);
}

void Vec2::Set(double x_, double y_) {
    x = x_;
    y = y_;
}

void Vec2::SetX(double x_) { x = x_; }

void Vec2::SetY(double y_) { y = y_; }

float Vec2::Xf(void) { return x; }

float Vec2::Yf(void) { return y; }

Vec2 operator+ (const Vec2& v1, const Vec2& v2)
{
    return Vec2(
        (v1.x + v2.x),
        (v1.y + v2.y));
}

Vec2 operator- (const Vec2& v1, const Vec2& v2)
{
    return Vec2(
        (v1.x - v2.x),
        (v1.y - v2.y));
}


Vec2 operator* (const Vec2& v1, double f)
{
    return Vec2(
        (v1.x * f),
        (v1.y * f));
}

double operator* (const Vec2& v1, const Vec2& v2)
{
    return (v1.x * v2.x) + (v1.y * v2.y);
}

float operator% (const Vec2& v1, const Vec2& v2)
{
    return (v1.x * v2.y) - (v1.y * v2.x);
}

bool operator== (const Vec2& v1, const Vec2& v2)
{
    return v1.x == v2.x && v1.y == v2.y;
}
