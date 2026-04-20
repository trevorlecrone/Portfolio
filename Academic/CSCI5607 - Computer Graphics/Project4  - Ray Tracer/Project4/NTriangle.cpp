#include "NTriangle.h"
#include <math.h>

double NTriangle::CheckCollision(Ray ray) {
    Vec r_dir = ray.GetDir();
    double denom = (r_dir*pNorm);
    if (fabs(denom) > 0.000001) {
        double t = -((ray.GetOrigin() * pNorm) - (v1 * pNorm)) / denom;
        if (t > 0) {
            if (BaryCoords(ray.GetOrigin() + (ray.GetDir()*t), bU, bV, bW)) {
                return t;
            }
            else {
                return -999;
            }
        }
        else {
            return -999;
        }
    }
    else {
        return -999;
    }
}
//function based off of Christer Ericson's Real-Time Collision Detection formula
//sets barycentric coords for computing the normal, since we will get the normal after detecting if a collision occurs
bool NTriangle::BaryCoords(Vec point, double &u_, double &v_, double &w_) {

    Vec p1 = v2 - v1,
        p2 = v3 - v1,
        p3 = point - v1;

    double dot11 = p1 * p1,
        dot12 = p1 * p2,
        dot22 = p2 * p2,
        dot31 = p3 * p1,
        dot32 = p3 * p2;

    double denom = (dot11 * dot22) - (dot12 * dot12);
    double v = ((dot22 * dot31) - (dot12 * dot32)) / denom;
    double w = ((dot11 * dot32) - (dot12 * dot31)) / denom;
    if ((v + w) <= 1 && (v + w) >= 0 && v <= 1 && w <= 1 && v >= 0 && w >= 0) {
        v_ = v;
        w_ = w;
        u_ = 1.0 - (v + w);
        return true;
    }
    else {
        return false;

    }
}

bool NTriangle::CheckShadow(Ray ray, double offset, double max) {
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

Vec NTriangle::GetNormal(Vec point) {
    Vec normal = (n1 * bU) + (n2 * bV) + (n3 * bW);
    return normal.Normalize();
}
