#include "Line.h"

Vec2 Line::GetAnchor2() {
    return anchor2;
}
void Line::SetAnchor2(Vec2 a2) {
    anchor2 = a2;
}

float* Line::VertexArray() {
    float* result = new float[8];
    result[0] = anchor.x;
    result[1] = anchor.y;
    result[2] = 0;
    result[3] = 0;
    result[4] = anchor2.x;
    result[5] = anchor2.y;
    result[6] = 0;
    result[7] = 0;
    return result;
}

int Line::VertexArrayFloats() {
    return 8;
}

int Line::Type() {
    return LINE_TYPE;
}