#pragma once
#include "Vec.h"
struct Camera {
public:

    Camera() : position(Vec(0, 0, 0)), up(Vec(0, 1, 0)), at(Vec(0,0,1)), angle(45){};
    Camera(Vec p) : position(p), up(Vec(0, 1, 0)), at(Vec(0, 0, 1)), angle(45) {};
    Camera(Vec p, double ang) : position(p), up(Vec(0, 1, 0)), at(Vec(0, 0, 1)), angle(ang) {};
    Camera(Vec p, Vec u, Vec a, double ang) : position(p), up(u.Normalize()), at(a.Normalize()), angle(ang) {
        if (up*at != 0) {
            up = up - (at *((at*up) / (up*up)));
        }
    };

    Vec position;
    Vec up;
    Vec at;
    double angle;
};