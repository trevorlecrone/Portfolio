#pragma once
#include"Vec.h"
#include "Light.h"

class Ray {
public:
    Ray() : origin(Vec()), dir(Vec()) {};
    Ray(Vec origin_, Vec dir_) : origin(origin_), dir(dir_) {};
    
    Vec GetOrigin();
    Vec GetDir();

    void SetOrigin(Vec origin_);
    void SetDir(Vec dir_);

protected:
    Vec origin;
    Vec dir;
};