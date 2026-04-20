#pragma once
#include "Circle.h"
#include "SceneObject.h"
struct StaticBumper : public SceneObject
{
public:
    StaticBumper(Circle* c, double elasticity_) : SceneObject(c, 0.0, elasticity_) {};

    int SpecialCollision() { return BUMPER_COLLISION; };

    bool IsStatic() { return true; };

    Vec2 Velocity() { return Vec2(0.0, 0.0); };

    void SetVelocity(Vec2 v) { return; };

    int isActive = 0;

    void Activate() { isActive = 24; };
};