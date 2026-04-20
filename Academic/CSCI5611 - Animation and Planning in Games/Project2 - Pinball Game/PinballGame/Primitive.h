#pragma once

#include "Vec2.h"
#include <stdlib.h>
#include <math.h>
#include <typeinfo>

class Primitive {

public:
    //Constructors
    //returns a Primitive with anchor and id set respectively;
    Primitive(Vec2 anchor_, int id_) : anchor(anchor_), id(id_) {};
    //returns a Primitive with standard unit vector and id 0;
    Primitive() : anchor(Vec2()), id(0) {};


    const int LINE_TYPE = 0;
    const int CIRCLE_TYPE = 1;
    const int BOX_TYPE = 2;

    Vec2 GetAnchor() {
        return anchor;
    }
    void SetAnchor(Vec2 a) {
        anchor = a;
    }

    const int GetID() const{
        return id;
    }

    virtual float* VertexArray() = 0;
    virtual int VertexArrayFloats() = 0;

    virtual int Type() = 0;

    virtual ~Primitive() {};

protected:
    Vec2 anchor;
    const int id;
};