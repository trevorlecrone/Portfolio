// Vector Structure with normalization, magnitude, projection, and angle finding functions
// operators overridden to perform relevant vector arithmetic, Based on 3D Vec library from 2017
// I wrote for ray tracer in 5607
// Trevor LeCrone 2023
#pragma once
#include <stdio.h>

struct Vec2
{
    //data
    double x, y;

    //Constructors
    //returns a Vec with x and y set respectively;
    Vec2(double x_, double y_) : x(x_), y(y_) {}
    //returns a standard 2D unit Vec in the +x direction
    Vec2() : x(1.0), y(0.0) {}

    //methods

    //setters
    void Set(double x_, double y_);
    void SetX(double x_);
    void SetY(double y_);

    //getters (float)
    float Xf(void);
    float Yf(void);

    //helpful functions

    double Magnitude();

    Vec2 Normalize();

    double FindAngle(Vec2 v2);

    Vec2 Reflect(Vec2 norm);

    Vec2 Project(Vec2 v2);

    void PrintMe();

};

// Addition of Vecs
Vec2 operator+ (const Vec2& v1, const Vec2& v2);

// Subtraction of Vecs
Vec2 operator- (const Vec2& v1, const Vec2& v2);

// Scalar multiplication
Vec2 operator* (const Vec2& v1, double f);

// Dot product
double operator* (const Vec2& v1, const Vec2& v2);

// Cross product
float operator% (const Vec2& v1, const Vec2& v2);

// equality
bool operator== (const Vec2& v1, const Vec2& v2);