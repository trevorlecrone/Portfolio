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

float* Circle::VertexArray() {
    float* result = new float[16];
    result[0] = anchor.x - radius;
    result[1] = anchor.y + radius;;
    result[2] = 0;
    result[3] = 0;
    result[4] = anchor.x + radius;;
    result[5] = anchor.y + radius;;
    result[6] = 1;
    result[7] = 0;
    result[8] = anchor.x - radius;;
    result[9] = anchor.y - radius;;
    result[10] = 0;
    result[11] = 1;
    result[12] = anchor.x + radius;;
    result[13] = anchor.y - radius;;
    result[14] = 1;
    result[15] = 1;
    return result;
}

int Circle::VertexArrayFloats() {
    return 16;
}

int Circle::Type() {
    return CIRCLE_TYPE;
}