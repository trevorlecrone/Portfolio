#pragma once
#include "Primitive.h"
struct CollisionData {
    CollisionData(Primitive* p1_, Primitive* p2_) : p1(p1_), p2(p2_) {}

public:
    Primitive* p1;
    Primitive* p2;
    bool HasPrimitive(int id) { return p1->GetID() == id || p2->GetID() == id; }
    int GetOtherPrimitive(int id) {
        if (p1->GetID() == id) {
            return p2->GetID();
        }
        else if (p2->GetID() == id) {
            return p1->GetID();
        }
        return -1;
    }
};