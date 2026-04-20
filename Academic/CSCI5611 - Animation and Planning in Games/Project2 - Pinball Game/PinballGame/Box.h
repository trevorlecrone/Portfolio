#pragma once
#include "Primitive.h"
#include "Line.h"
class Box : public Primitive
{
public:
    Box(Vec2 anchor_, double width_, double height_, int id) : Primitive(anchor_, id), width(width_), height(height_) {};

    double GetWidth();
    double GetHeight();

    float* VertexArray();
    int VertexArrayFloats();
    int Type();

    Line** GetComponentLines();

    Vec2 GetUpperLeft();
    Vec2 GetUpperRight();

protected:
    double width;
    double height;
};