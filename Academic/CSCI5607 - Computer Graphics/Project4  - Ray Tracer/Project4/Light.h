#pragma once
#include "Vec.h"
#include <stdio.h>

class Light {
public:
    Light() :  intensity(Vec()) {};
    Light(Vec intensity_) : intensity(intensity_){};

    void SetIntensity(Vec intensity_) {
        intensity = intensity_;
    };

    Vec GetIntensity() {
        return intensity;
    };

    virtual Vec IntensityAt(Vec hit) { return Vec(); };
    virtual Vec GetDirection(Vec hit) { return Vec(); };
    virtual double GetDistance(Vec hit) { return -999.0; };

protected:
    Vec intensity;
};