#pragma once
#include "Light.h"

class DLight : public Light
{
public:
    DLight() : Light(Vec()), direction(Vec()) {};
    DLight(Vec direction_, Vec intensity_) : Light(intensity_), direction(direction_.Normalize()) {};

    void SetDirection(Vec position);

    virtual Vec GetDirection(Vec hit);
    virtual Vec IntensityAt(Vec hit) { return intensity; };
    virtual double GetDistance(Vec hit) { return -999.0; };
protected:
    Vec direction;
};