#include "Material.h"   


Vec Material::GetAmbient() {
    return ambient;
}
Vec Material::GetDiffuse() {
    return diffuse;
}
Vec Material::GetSpecular() {
    return specular;
}
Vec Material::GetTransmissive() {
    return transmissive;
}
double Material::GetPhong() {
    return phong;
}
double Material::GetIor() {
    return ior;
}
Vec Material::Refract(Vec norm, Vec l) {
    double ang_inc = (l * norm);
    double ior_ratio;
    Vec reflect = l.Reflect(norm).Normalize();
    if (ang_inc < 0) {
        ang_inc = -ang_inc;
        ior_ratio = 1.0 / ior;
    }
    else {
        ior_ratio = ior;
        norm = norm*-1;
    }
    double dir_factor = 1 - ((ior_ratio * ior_ratio) * (1 - (ang_inc * ang_inc)));
    if (dir_factor < 0) {
        return reflect;
    }
    else {
        Vec l_comp = l * ior_ratio;
        Vec n_comp = norm * (ior_ratio * ang_inc - sqrt(dir_factor));
        (l_comp + n_comp).Normalize();
        return (l_comp + n_comp).Normalize();
    }
}

void Material::SetPhong(double p) {
    phong = p;
}
void Material::SetIor(double i) {
    ior = i;
}
void Material::SetAmbient(Vec amb) {
    ambient = amb;
}
void Material::SetDiffuse(Vec diff) {
    diffuse = diff;
}
void Material::SetTransmissive(Vec trans) {
    diffuse = trans;
}
void Material::SetSpecular(Vec spec) {
    specular = spec;
}