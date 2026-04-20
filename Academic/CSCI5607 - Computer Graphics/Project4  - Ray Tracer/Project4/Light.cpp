#include "Light.h"
#include <math.h>
#include <stdlib.h>

void Light::SetIntensity(Vec intensity_) {
    intensity = intensity_;
}

Vec Light::GetIntensity() {
    return intensity;
}