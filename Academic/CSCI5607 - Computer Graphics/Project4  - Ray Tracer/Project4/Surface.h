//3d Surface super class
//Trevor LeCrone 2017
#pragma once
#include <stdlib.h>
#include <math.h>
#include "Ray.h"
#include "Material.h"

class Surface {

public:
    Surface() : center(Vec()), mat(Material()) {};
    Surface(Vec center_, Material mat_) : center(center_), mat(mat_) {};

    
    Vec GetCenter() {
        return center;
    }
    void SetCenter(Vec c) {
        center = c;
    }

    Material GetMaterial() {
        return mat;
    }
    void SetMaterial(Material mat_) {
        mat = mat_;
    }


    virtual double CheckCollision(Ray ray){ return  0.0;};
    virtual bool CheckShadow(Ray ray, double offset, double max_t) { return false; };

    virtual Vec GetNormal(Vec point) { return Vec(); };

    double quadratic(double a, double b, double c) {
        double discriminant = (b * b) - (4 * a * c);
        //printf("discriminant: %f\n", discriminant);
        if (discriminant < 0) {
            return -999;
        }
        //if we are tangent
        else if (discriminant == 0) {
            return b / (2 * a);
        }
        else {
            double root;
            if (b >= 0) {
                root = (-b + sqrt(discriminant)) / 2;
            }
            else {
                root = (-b - sqrt(discriminant)) / 2;
            }
            double intersection_1 = root / a;
            double intersection_2 = c / root;
            double t_val = fmin(intersection_1, intersection_2);
            if (t_val > 0) {
                return t_val;
            }
            else {
                return -999;
            }
        }

    }

    virtual ~Surface() {};

protected:
    Vec center;
    Material mat;
};