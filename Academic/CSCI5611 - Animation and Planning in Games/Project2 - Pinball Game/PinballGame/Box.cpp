#include "Box.h"

double Box::GetWidth() {
    return width;
}
double Box::GetHeight() {
    return height;
}

float* Box::VertexArray() {
    float* result = new float[16];
    result[0] = anchor.x - width / 2;
    result[1] = anchor.y + height / 2;
    result[2] = 0;
    result[3] = 0;
    result[4] = anchor.x + width / 2;
    result[5] = anchor.y + height / 2;
    result[6] = 1;
    result[7] = 0;
    result[8] = anchor.x - width / 2;
    result[9] = anchor.y - height / 2;
    result[10] = 0;
    result[11] = 1;
    result[12] = anchor.x + width / 2;
    result[13] = anchor.y - height / 2;
    result[14] = 1;
    result[15] = 1;
    return result;
}

int Box::VertexArrayFloats() {
    return 16;
}

Line** Box::GetComponentLines() {
    Line* lines[4];
    lines[0] = new Line(Vec2(anchor.x - width, anchor.y + height), Vec2(anchor.x + width, anchor.y + height), 999);
    lines[1] = new Line(Vec2(anchor.x + width, anchor.y + height), Vec2(anchor.x + width, anchor.y - height), 999);
    lines[2] = new Line(Vec2(anchor.x + width, anchor.y - height), Vec2(anchor.x - width, anchor.y - height), 999);
    lines[3] = new Line(Vec2(anchor.x - width, anchor.y - height), Vec2(anchor.x - width, anchor.y + height), 999);
    return lines;
}

Vec2 Box::GetUpperLeft() {
    return Vec2(anchor.x - width, anchor.y + height);
}

Vec2 Box::GetUpperRight() {
    return Vec2(anchor.x + width, anchor.y + height);
}

int Box::Type() {
    return BOX_TYPE;
}