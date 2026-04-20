#pragma once
#include "Light.h"

class SLight : public Light
{
public:
    SLight() : Light(Vec()), position(Vec()) {};
    SLight(Vec position_, Vec intensity_, Vec direction_, double ang1, double ang2) : Light(intensity_), position(position_), angle1(ang1), angle2(ang2), direction(direction) {};

    void SetPosition(Vec position);

    Vec GetPosition();
    virtual Vec GetDirection();
    virtual Vec IntensityAt(Vec hit);
    virtual double GetDistance(Vec hit);

protected:
    Vec position;
    double angle1;
    double angle2;
    Vec direction;
};