#pragma once
#include "Circle.h"
#include "SceneObject.h"
struct BlackHole : public SceneObject
{
public:
    BlackHole(Circle* c, double renderRadius_, Vec2 whiteHoleCoords_) : SceneObject(c, 0.0, 1.0), renderRadius(renderRadius_), whiteHoleCoords(whiteHoleCoords_) {};

    int SpecialCollision() { return BLACK_HOLE_COLLISION; };

    bool IsStatic() { return true; };

    Vec2 Velocity() { return Vec2(0.0, 0.0); };

    void SetVelocity(Vec2 v) { return; };

    double renderRadius;
    Vec2 whiteHoleCoords;
};