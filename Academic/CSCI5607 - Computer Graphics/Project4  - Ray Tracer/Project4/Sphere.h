#pragma once
#include "Surface.h"
class Sphere : public Surface
{
public:
    Sphere(Vec center_, double radius_, Material mat_) : Surface(center_, mat_), radius(radius_) {};

    virtual double CheckCollision(Ray ray);
    virtual bool CheckShadow(Ray ray, double offset, double max_t);
    virtual Vec GetNormal(Vec point);
    
    double GetRadius();

protected:
    double radius;
};