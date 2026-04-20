#pragma once
#include "Primitive.h"
class Line : public Primitive
{
public:
    Line(Vec2 anchor_, Vec2 anchor2_, int id) : Primitive(anchor_, id), anchor2(anchor2_) {};

    Vec2 GetAnchor2();
    void SetAnchor2(Vec2 a2);

    float* VertexArray();
    int VertexArrayFloats();
    int Type();

protected:
    Vec2 anchor2;
};