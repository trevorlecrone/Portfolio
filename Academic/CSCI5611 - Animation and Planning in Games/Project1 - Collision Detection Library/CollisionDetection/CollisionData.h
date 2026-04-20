#pragma once
#include "Primitive.h"
struct CollisionData {
    CollisionData(Primitive p1_, Primitive p2_) : p1(p1_), p2(p2_) {}

public:
    Primitive p1;
    Primitive p2;
};