#pragma once
#include "SceneObject.h"
#include "Pinball.h"

using namespace std;
class CollisionHandler
{
public:
    static void ResolveCollision(Pinball* o1, SceneObject* o2, int mode, int& score);
};