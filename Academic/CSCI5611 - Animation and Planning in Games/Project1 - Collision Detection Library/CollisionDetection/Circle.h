#pragma once
#include "Primitive.h"
class Circle : public Primitive
{
public:
    Circle(Vec2 anchor_, double radius_, int id) : Primitive(anchor_, id), radius(radius_) {};

    double GetRadius();

    double Quadratic(double a, double b, double c);

protected:
    double radius;
};