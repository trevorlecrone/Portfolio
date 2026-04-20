#include "Ray.h"


Vec Ray::GetOrigin() {
    return origin;
}
Vec Ray::GetDir() {
    return dir;
}

void Ray::SetOrigin(Vec origin_) {
    origin = origin_;
}
void Ray::SetDir(Vec dir_) {
    dir = dir_;
}
