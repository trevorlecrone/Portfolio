#include "PLight.h"
#include <math.h>
#include <stdlib.h>

void PLight::SetPosition(Vec position_) {
    position = position_;
}

Vec PLight::GetPosition() {
    return position;
}

Vec PLight::GetDirection(Vec hit) {
    return position - hit;
}

Vec PLight::IntensityAt(Vec hit) {
    double distance = (hit - position).Magnitude();
    distance += (.6 * distance*distance);
    Vec i_at_p = Vec((intensity.X() / distance),
        (intensity.Y() / distance),
        (intensity.Z() / distance));
    return i_at_p;
}
double PLight::GetDistance(Vec hit) {
    double distance = (hit - position).Magnitude();
    return distance;
}