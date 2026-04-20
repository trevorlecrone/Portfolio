#pragma once
#include <set>
#include <chrono>
#include <vector>
#include <map>
#include "Circle.h"
#include "Box.h"
#include "Line.h"
#include "CollisionData.h"

using namespace std;
using std::chrono::high_resolution_clock;
using std::chrono::duration;
using std::chrono::milliseconds;
class DetectionLib
{
public:
    static void DetectCollisions(vector<Circle*> circles, vector<Line*> lines, vector<Box*> boxes, vector<CollisionData>& collisions);

    static void DebugDetectCollisions(vector<Circle*> circles, vector<Line*> lines, vector<Box*> boxes, vector<CollisionData>& collisions, map<int, duration<double, std::milli>>& debugMap);
};