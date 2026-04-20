#pragma once
#include "Circle.h"
#include "SceneObject.h"
struct Pinball : public SceneObject
{
public:
    Pinball(Circle* c, double mass_, double elasticity_, Vec2(velocity_)) : SceneObject(c, mass_, elasticity_), velocity(velocity_){};

    Vec2 velocity;

    int SpecialCollision() { return 0; };

    bool IsStatic() { return false; };

    Vec2 Velocity() { return velocity; };

    void SetVelocity(Vec2 v) { velocity = v; };

    bool locked;
};