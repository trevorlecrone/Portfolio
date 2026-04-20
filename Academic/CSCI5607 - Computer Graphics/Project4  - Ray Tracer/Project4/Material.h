#pragma once
#include <stdio.h>
#include <cstdio>
#include <math.h>
#include "Vec.h"
class Material {

public:
    Material() : ambient(Vec(0, 0, 0)), diffuse(Vec(1,1,1)), specular(Vec(0, 0, 0)), transmissive(Vec(0,0,0)), phong(5.0), ior(1.0) {};
    Material(Vec diff) : ambient(Vec(0,0,0)), diffuse(diff), specular(Vec(0,0,0)), transmissive(Vec(0, 0, 0)), phong(5.0), ior(1.0) {};
    Material(Vec diff, double phong_) : ambient(Vec(0, 0, 0)), diffuse(diff), specular(Vec(0, 0, 0)), transmissive(Vec(0, 0, 0)), phong(phong_), ior(1.0) {};
    Material(Vec diff, Vec spec, double phong_) : ambient(Vec(0, 0, 0)), diffuse(diff), specular(spec), transmissive(Vec(0, 0, 0)), phong(phong_), ior(1.0) {};
    Material(Vec diff, Vec amb, Vec spec, double phong_) : ambient(amb), diffuse(diff), specular(spec), transmissive(Vec(0, 0, 0)), phong(phong_), ior(1.0) {};
    Material(Vec diff, Vec amb, Vec spec, Vec trans, double phong_, double ior_) : ambient(amb), diffuse(diff), specular(spec), transmissive(trans), phong(phong_), ior(ior_) {};


    Vec GetAmbient();
    Vec GetDiffuse();
    Vec GetSpecular();
    Vec GetTransmissive();
    double GetPhong();
    double GetIor();
    Vec Refract(Vec norm, Vec l);

    void SetAmbient(Vec amb);
    void SetDiffuse(Vec diff);
    void SetSpecular(Vec spec);
    void SetTransmissive(Vec spec);
    void SetPhong(double p);
    void SetIor(double i);


protected:
    double phong;
    double ior;
    Vec ambient;
    Vec diffuse;
    Vec specular;
    Vec transmissive;
};



