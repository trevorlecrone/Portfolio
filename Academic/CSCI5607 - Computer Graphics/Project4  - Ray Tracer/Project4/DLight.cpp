#include "DLight.h"
#include <math.h>
#include <stdlib.h>

void DLight::SetDirection(Vec direction_) {
    direction = direction_;
}

Vec DLight::GetDirection(Vec hit) {
    return (direction.Normalize() * -1);
}
