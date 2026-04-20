#include "Circle.h"
#include "Box.h"
#include "Line.h"
#include "Pinball.h"
#include "StaticBumper.h"
#include "BlackHole.h"
#include "WhiteHole.h"
#include "Flipper.h"
#include "CollisionHandler.h"
#include <set>
#include <map>
#include <cstring>
#include <cstdlib>
#include <vector>
using namespace std;

Vec2 GetCircleCircleCollisionVectorAndHandleOverlap(Circle* c1, Circle* c2, bool isStatic, bool isBH) {
    double radii = c1->GetRadius() + c2->GetRadius();
    Vec2 dir = c1->GetAnchor() - c2->GetAnchor();
    if (isBH) {
        //just want direction when we are doing black hole calculations
        return dir;
    }
    double dist = dir.Magnitude();
    Vec2 dirNorm = dir.Normalize();
    if (dist < radii) {
        // the objects are both pinballs, move them both
        if (!isStatic) {
            double overlap = (radii - dist) / 2;
            c1->SetAnchor(c1->GetAnchor() + dirNorm * (overlap));
            c2->SetAnchor(c2->GetAnchor() - dirNorm * (overlap));
        }
        else {
            double overlap = (radii - dist);
            c1->SetAnchor(c1->GetAnchor() + dirNorm * (overlap));
        }
    }
    return dir;
}

Vec2 GetCircleLineCollisionPointAndHandleOverlap(Circle* c, Line* l, Vec2& closest) {
    Vec2 closestPoint;
    Vec2 dir = l->GetAnchor2() - l->GetAnchor();
    Vec2 centerToAnchor = c->GetAnchor() - l->GetAnchor();
    Vec2 dirNorm = dir.Normalize();
    double projected = dir.Normalize() * centerToAnchor;
    if (projected < 0) {
        closestPoint = l->GetAnchor();
    }
    else if (projected > dir.Magnitude()) {
        closestPoint = l->GetAnchor2();
    }
    else {
        closestPoint = l->GetAnchor() + dirNorm * projected;
    }
    closest = closestPoint;
    Vec2 lineNorm = c->GetAnchor() - closestPoint;
    double dist = lineNorm.Magnitude();
    lineNorm = lineNorm.Normalize();
    if (dist < c->GetRadius()) {
        c->SetAnchor(closestPoint + lineNorm * (c->GetRadius() + 0.00001));
    }
    return lineNorm;
}

void ApplyFlipperVelocity(Pinball* pb, Flipper* f, Vec2 vel, Vec2 dir) {
    Vec2 ballV = pb->Velocity();
    double ballVProj = pb->Velocity() * dir;
    double flipperV = vel * dir;
    double m1 = pb->mass;
    double m2 = f->mass;

    double ballVNew = (m1 * ballVProj + m2 * flipperV - m2 * (ballVProj - flipperV) * (pb->elasticity * f->elasticity)) / (m1 + m2);
    double ballVDiff = ballVNew - ballVProj;
    pb->SetVelocity(ballV + (dir * ballVDiff));
}

void ResolvePinballBumper(Pinball* o1, StaticBumper* o2, int mode, int& score) {
    Circle* c1 = (Circle*)o1->p;
    Circle* c2 = (Circle*)o2->p;
    Vec2 dirNorm = GetCircleCircleCollisionVectorAndHandleOverlap(c1, c2, o2->IsStatic(), false).Normalize();
    Vec2 v1 = o1->Velocity();
    Vec2 newV = v1.Reflect(dirNorm);
    //reactive bumper, adds velocity to ball
    if (o2->elasticity > 1.0) {
        o1->SetVelocity(newV + (dirNorm * (o2->elasticity - 1.0)));
        o2->Activate();
        if (mode) {
            score += 100;
        }
    }
    else {
        o1->SetVelocity(newV * (o1->elasticity * o2->elasticity));
    }
}

void ResolvePinballPinball(Pinball* o1, Pinball* o2) {
    Circle* c1 = (Circle*)o1->p;
    Circle* c2 = (Circle*)o2->p;
    Vec2 dirNorm = GetCircleCircleCollisionVectorAndHandleOverlap(c1, c2, o2->IsStatic(), false).Normalize();
    Vec2 v1 = o1->Velocity();
    Vec2 v2 = o2->Velocity();
    double vel1 = v1 * dirNorm;
    double vel2 = v2 * dirNorm;
    double m1 = o1->mass;
    double m2 = o2->mass;
    double vel1New = (m1 * vel1 + m2 * vel2 - m2 * (vel1 - vel2) * (o1->elasticity * o2->elasticity)) / (m1 + m2);
    double vel2New = (m1 * vel1 + m2 * vel2 - m1 * (vel2 - vel1) * (o1->elasticity * o2->elasticity)) / (m1 + m2);
    double v1Diff = vel1New - vel1;
    double v2Diff = vel2New - vel2;
    o1->SetVelocity(v1 + (dirNorm * v1Diff));
    o2->SetVelocity(v2 + (dirNorm * v2Diff));
}

void ResolvePinballBlackHole(Pinball* o1, BlackHole* o2) {
    Circle* c1 = (Circle*)o1->p;
    Circle* c2 = (Circle*)o2->p;
    //want to pull ball into black hole, hence order of args
    Vec2 dir = GetCircleCircleCollisionVectorAndHandleOverlap(c2, c1, o2->IsStatic(), true);
    Vec2 dirNorm = dir.Normalize();
    double distance = dir.Magnitude();
    Vec2 v1 = o1->Velocity();
    o1->SetVelocity(v1 + dirNorm * (1 / (2 * distance)));
    Circle* renderC = new Circle(c2->GetAnchor(), o2->renderRadius, c2->GetID());
    //if colliding with the rendered object, teleport to white hole with new velocity
    Vec2 renderDist = GetCircleCircleCollisionVectorAndHandleOverlap(c1, c2, o2->IsStatic(), true);
    double radii = c1->GetRadius() + renderC->GetRadius();
    if (renderDist * renderDist < (radii * radii)/1.2) {
        c1->SetAnchor(o2->whiteHoleCoords);
    }
}

void ResolvePinballFlipper(Pinball* o1, Flipper* o2) {
    Circle* c = (Circle*)o1->p;
    Line* l = (Line*)o2->p;
    Vec2 closest;
    Vec2 dir = GetCircleLineCollisionPointAndHandleOverlap(c, l, closest);
    Vec2 radius = closest - o2->flipperAnchor;
    Vec2 vel = Vec2(-radius.y, radius.x) * o2->rotationalVelocity;
    ApplyFlipperVelocity(o1, o2, vel, dir);
    return;
    
}

void ResolvePinballLine(Pinball* o1, SceneObject* o2) {
    Circle* c = (Circle*)o1->p;
    Line* l = (Line*)o2->p;
    Vec2 closest;
    Vec2 lineNorm = GetCircleLineCollisionPointAndHandleOverlap(c, l, closest);
    double oldMag = o1->Velocity().Magnitude();
    Vec2 newV = o1->Velocity().Reflect(lineNorm) * (o1->elasticity * o2->elasticity);
    o1->SetVelocity(newV);
}

void CollisionHandler::ResolveCollision(Pinball* o1, SceneObject* o2, int mode, int& score)
{
    if (o2->IsStatic()) {
        if (o2->SpecialCollision() == o2->BUMPER_COLLISION) {
            ResolvePinballBumper(o1, (StaticBumper*)o2, mode, score);
        }
        else if (o2->SpecialCollision() == o2->BLACK_HOLE_COLLISION) {
            ResolvePinballBlackHole(o1, (BlackHole*)o2);
        }
        else if (o2->SpecialCollision() == o2->FLIPPER_COLLISION) {
            ResolvePinballFlipper(o1, (Flipper*)o2);
        }
        else if (o2->SpecialCollision() == o2->NO_OP_COLLISION) {
            return;
        }
        else {
            ResolvePinballLine(o1, o2);
        }
    }
    else {
        ResolvePinballPinball(o1, (Pinball*)o2);
    }

}


