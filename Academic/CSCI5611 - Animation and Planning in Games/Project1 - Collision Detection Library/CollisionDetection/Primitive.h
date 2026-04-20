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


    Vec2 GetAnchor() {
        return anchor;
    }
    void SetAnchor(Vec2 a) {
        anchor = a;
    }

    const int GetID() const{
        return id;
    }

    virtual ~Primitive() {};

protected:
    Vec2 anchor;
    const int id;
};