//Vector Struction with normalization, magnitude, projection, and angle finding functions
//operators overidden to perform relevant vector arithmatic.
//Trevor LeCrone 2017
#pragma once
#include <stdio.h>

struct Vec
{
    //data
    double x, y, z;

    //Constructors
    //returns a Vec with x y and z set respectively;
    Vec(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}
    //returns a standard 3D unit Vec in the +x direction
    Vec() : x(1.0), y(0.0), z(0.0) {}

    //methods

    //setters
    void Set(double x_, double y_, double z_);
    void SetX(double x_);
    void SetY(double y_);
    void SetZ(double z_);

    //getters
    double X(void);
    double Y(void);
    double Z(void);

    //helpful functions

    double Magnitude ();
    
    Vec Normalize();

    double FindAngle(Vec v2);

    Vec Reflect(Vec norm);

    Vec Project(Vec v2);

    void PrintMe();

};

    //returns a unit length Vec in the same direction as the original
    //the original Vec stays the same when this function is called
    
    

    // Addition of Vecs
    Vec operator+ (const Vec& v1, const Vec& v2);

    // subtraction of Vecs
    Vec operator- (const Vec& v1, const Vec& v2);

    // Scalar multiplication
    Vec operator* (const Vec& v1, double f);

    // Dot product
    double operator* (const Vec& v1, const Vec& v2);

    // Cross product
    Vec operator% (const Vec& v1, const Vec& v2);