#pragma once

#include "Primitive.h"
#include "Circle.h"
#include <stdlib.h>
#include <math.h>

struct SceneObject {

public:
    //Constructors
    //returns a SceneObject primitive, mass and elasticity set accordingly;
    SceneObject(Primitive* p_, double mass_, double elasticity_) : p(p_), id(p_->GetID()), mass(mass_), elasticity(elasticity_) {};
    //returns a SceneObject with circle and mass;
    SceneObject() : p(new Circle(Vec2(), 1.0, 0)), id(0), mass(1.0), elasticity(1.0) {};

    constexpr static int BUMPER_COLLISION = 1;
    constexpr static int BLACK_HOLE_COLLISION = 2;
    constexpr static int SPEED_MULTIPLIER_COLLISION = 3;
    constexpr static int FLIPPER_COLLISION = 4;
    constexpr static int NO_OP_COLLISION = 5;

    Primitive* p;
    int id;
    double mass;
    double elasticity;

    int GetID() { return p->GetID(); }

    virtual int SpecialCollision() = 0;
    virtual bool IsStatic() = 0;
    virtual Vec2 Velocity() = 0;
    virtual void SetVelocity(Vec2 v) = 0;

    virtual ~SceneObject() {};
};