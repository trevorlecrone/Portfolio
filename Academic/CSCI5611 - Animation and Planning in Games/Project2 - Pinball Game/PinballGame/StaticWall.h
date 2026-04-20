#pragma once
#include "Line.h"
#include "SceneObject.h"
struct StaticWall : public SceneObject
{
public:
    StaticWall(Line* l, double elasticity_) : SceneObject(l, 0.0, elasticity_){};

    int SpecialCollision() { return 0; };

    bool IsStatic() { return true; };

    Vec2 Velocity() { return Vec2(0.0,0.0); };

    void SetVelocity(Vec2 v) { return; };
};