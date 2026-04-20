#pragma once
#include "Surface.h"
//Triangle with normals assigned to each vertex

class NTriangle : public Surface
{

public:
    NTriangle() : v1(Vec(-0.5, 0, 0)), v2(Vec(0.5, 0, 0)), v3(Vec(0, 1, 0)) {
        Vec dVec1 = (v3 - v1).Normalize();
        Vec dVec2 = (v2 - v1).Normalize();
    };

    NTriangle(Vec v1_, Vec v2_, Vec v3_, Vec n1_, Vec n2_, Vec n3_, Vec origin, Material mat) : Surface(origin, mat), v1(v1_), v2(v2_), v3(v3_), n1(n1_), n2(n2_), n3(n3_) {
        Vec dVec1 = (v3 - v1);
        Vec dVec2 = (v2 - v1);
        pNorm = dVec1 % dVec2;
        bU = -1;
        bV = -1;
        bW = -1;
    };

    virtual double CheckCollision(Ray ray);
    virtual bool CheckShadow(Ray ray, double offset, double max_t);
    virtual Vec GetNormal(Vec point);
    bool BaryCoords(Vec point, double &u, double &v, double &w);

//we will recycle the center attribute of the surface as the origin we calculate distance from;
protected:
    Vec v1;
    Vec v2;
    Vec v3;
    Vec n1;
    Vec n2;
    Vec n3;
    Vec pNorm;
    double bU, bV, bW;
};