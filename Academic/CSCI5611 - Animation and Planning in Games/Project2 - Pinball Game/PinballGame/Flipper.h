#pragma once
#include "Box.h"
#include "SceneObject.h"
using namespace std;
#define PI 3.14159265

struct Flipper : public SceneObject
{
public:
    //TODO make several lines if time allows, or use a normal to the line as a buffer to simulate a box
    Flipper(Line* l, double mass_, double elasticity_, Vec2 flipperAnchor_, double angle_) : SceneObject(l, mass, elasticity_), flipperAnchor(flipperAnchor_), rotationalVelocity(0.0), angle(angle_){
        distance = (l->GetAnchor2() - l->GetAnchor()).Magnitude();
        double sign = l->GetAnchor2().x - l->GetAnchor().x > 0.0 ? 1.0 : -1.0;
        double anchor2x = l->GetAnchor().x + (sign * distance * cos(angle));
        double anchor2y = l->GetAnchor().y + (sign * distance * sin(angle));
        l->SetAnchor2(Vec2(anchor2x, anchor2y));
        finish = -angle;
    };
    Flipper() : SceneObject(), flipperAnchor(Vec2()), rotationalVelocity(0.0), angle(0.0), distance(1.0), finish(0.0) {};
    int SpecialCollision() { return FLIPPER_COLLISION; };

    bool IsStatic() { return true; };

    Vec2 Velocity() { return Vec2(0.0, 0.0); };

    void SetVelocity(Vec2 v) { return; };

    void Rotate() { 
        if (!rotationalVelocity) {
            return;
        }
        if (angle != finish) {
            angle += rotationalVelocity;
            //clamp to our limits
            angle = angle < -PI / 6.0 ? -PI / 6.0 : angle > PI / 6.0 ? PI / 6.0 : angle;
            Line* l = (Line*)p;
            double sign = l->GetAnchor2().x - l->GetAnchor().x > 0.0 ? 1.0 : -1.0;
            double anchor2x = l->GetAnchor().x + (sign * distance * cos(angle));
            double anchor2y = l->GetAnchor().y + (sign * distance * sin(angle));
            l->SetAnchor2(Vec2(anchor2x, anchor2y));
        }
        else {
            rotationalVelocity = 0.0;
            finish = -finish;
        }
    };

    Vec2 flipperAnchor;
    double distance;
    double rotationalVelocity;
    double angle;
    double finish;

    Flipper& operator= (const Flipper&) = default;
};
