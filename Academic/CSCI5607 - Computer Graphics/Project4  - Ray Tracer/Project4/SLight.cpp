#include "SLight.h"
#include <math.h>
#include <stdlib.h>

void SLight::SetPosition(Vec position_) {
    position = position_;
}

Vec SLight::GetPosition() {
    return position;
}

Vec SLight::GetDirection() {
    return direction;
}

Vec SLight::IntensityAt(Vec hit) {
    double distance = (hit - position).Magnitude();
    distance += (.2 * distance*distance);
    double angle = acos((hit - position) * direction);
    Vec i_at_p = Vec((intensity.X() / distance),
        (intensity.Y() / distance),
        (intensity.Z() / distance));
    if (angle <= angle1) {
        return i_at_p;
    }
    else if (angle < angle2) {
        return Vec(0, 0, 0);
    }
    else {
        double falloff = (angle - angle1) / (angle2 - angle1);
        return i_at_p*falloff;
    }
}
double SLight::GetDistance(Vec hit) {
    double distance = (hit - position).Magnitude();
    return distance;
}