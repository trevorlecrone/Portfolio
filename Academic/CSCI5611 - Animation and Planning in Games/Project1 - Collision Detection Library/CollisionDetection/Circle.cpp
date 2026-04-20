#include "Circle.h"
#include <math.h>

double Circle::GetRadius() {
    return radius;
}

double Circle::Quadratic(double a, double b, double c) {
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