#pragma once
#include "Light.h"

class PLight : public Light
{
public:
    PLight() : Light(Vec()), position(Vec()) {};
    PLight(Vec position_, Vec intensity_) : Light(intensity_), position(position_) {};

    void SetPosition(Vec position);

    Vec GetPosition();
    virtual Vec GetDirection(Vec hit);
    virtual Vec IntensityAt(Vec hit);
    virtual double GetDistance(Vec hit);

protected:
    Vec position;
};