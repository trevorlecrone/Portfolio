#pragma once
#include "Circle.h"
#include "SceneObject.h"
struct WhiteHole : public SceneObject
{
public:
    WhiteHole(Circle* c) : SceneObject(c, 0.0, 1.0) {};

    int SpecialCollision() { return NO_OP_COLLISION; };

    bool IsStatic() { return true; };

    Vec2 Velocity() { return Vec2(0.0, 0.0); };

    void SetVelocity(Vec2 v) { return; };
};