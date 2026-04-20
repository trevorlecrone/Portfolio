#pragma once
#include "Primitive.h"
class Box : public Primitive
{
public:
    Box(Vec2 anchor_, double width_, double height_, int id) : Primitive(anchor_, id), width(width_), height(height_) {};

    double GetWidth();
    double GetHeight();

protected:
    double width;
    double height;
};