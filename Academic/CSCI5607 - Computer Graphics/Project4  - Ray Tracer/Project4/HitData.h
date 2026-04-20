#pragma once
#include"Scene.h"

struct HitData {
    HitData() : mat(Material()), point(Vec()), s_normal(Vec()), s(Scene()), ray(Ray()) {};
    HitData(Material mat_, Vec point_, Vec s_normal_, Scene s_, Ray ray_) : mat(mat_), point(point_), s_normal(s_normal_), s(s_), ray(ray_) {};

public:
    Material mat;
    Ray ray;
    Vec point;
    Vec s_normal;
    Scene s;
};