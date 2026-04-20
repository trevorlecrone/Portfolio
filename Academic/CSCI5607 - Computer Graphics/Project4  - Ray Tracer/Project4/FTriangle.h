#pragma once
#include "Surface.h"

class FTriangle : public Surface 
{

public:
    FTriangle() : v1(Vec(-0.5,0,0)), v2(Vec(0.5,0,0)), v3(Vec(0,1,0)) {
        Vec dVec1 = (v3 - v1).Normalize();
        Vec dVec2 = (v2 - v1).Normalize();
        center = Vec(0, 0, 0);
        normal = dVec1 % dVec2;
        if (((v1 - center) * normal) > 0) {
            normal = (normal * -1);
        }
    };

    FTriangle(Vec v1_, Vec v2_, Vec v3_, Vec origin, Material mat) : Surface(origin, mat), v1(v1_), v2(v2_), v3(v3_) {
        Vec dVec1 = (v3 - v1);
        Vec dVec2 = (v2 - v1);
        normal = dVec1 % dVec2;
        if (((v1 - center) * normal) > 0) {
            normal = (normal * -1);
        }
    };

    virtual double CheckCollision(Ray ray);
    virtual bool CheckShadow(Ray ray, double offset, double max_t);
    virtual Vec GetNormal(Vec point) { return normal.Normalize(); };
    bool BaryCheck(Vec point);

//we will recycle the center attribute of the surface as the origin we calculate distance from;
protected:
    Vec v1;
    Vec v2;
    Vec v3;
    Vec normal;
};