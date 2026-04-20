#include "FTriangle.h"
#include <math.h>

double FTriangle::CheckCollision(Ray ray) {
    Vec r_dir = ray.GetDir();
    double denom = (r_dir*normal);
    if (fabs(denom) > 0.0000001) {
        double t = -((ray.GetOrigin() * normal) - (v1 * normal) ) / denom;
        if (t > 0) {
            if (BaryCheck(ray.GetOrigin() + (ray.GetDir()*t))) {
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
bool FTriangle::BaryCheck(Vec point) {
   /* bool side1 = SameSide(point, v1, v2, v3);
    bool side2 = SameSide(point, v2, v1, v3);
    bool side3 = SameSide(point, v3, v1, v2);
    if (side1 && side2 && side3) {
        return true;
    }
    else {
        return false;
    }
    */
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
    if ((v + w) <= 1 && (v + w) >= 0 && v <= 1 && w <= 1 && v >= 0 &&  w >= 0) {
        return true;
    }
    else {
        return false;

    }


}

bool FTriangle::CheckShadow(Ray ray, double offset, double max) {
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

