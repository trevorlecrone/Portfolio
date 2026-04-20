#include "DetectionLib.h"
#include "Circle.h"
#include "Box.h"
#include "Line.h"
#include "CollisionData.h"
#include <set>
#include <map>
#include <typeinfo>
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <chrono>
#include <vector>
using namespace std;
static const size_t LINE_TYPE = typeid(Line).hash_code();
static const size_t CIRCLE_TYPE = typeid(Circle).hash_code();
static const size_t BOX_TYPE = typeid(Box).hash_code();

//Based off approach in "Line Segment Intersection" by EngineerNick on Youtube, his example python has signs mixed up though
// and did not consider collinear segments to be intersecting. Used in Line-Line and Line-Box check
bool SegmentCollisionTest(Vec2 seg1, Vec2 seg2, Vec2 anchorDiff) {
    double determinate = seg1 % seg2;
    //segments are collinear on some ray or parallel
    if (determinate == 0) {
        //if the difference in anchors is also parallel, we are collinear on some ray, check to see if segments connect
        if (anchorDiff % seg1 == 0) {
            Vec2 fullSeg = seg2 - seg1;
            if (anchorDiff.x <= fullSeg.x && anchorDiff.y <= fullSeg.y) {
                return true;
            }
        }
        return false;
    }
    else {
        // compute that the point the Ray defining the lines direction actually occurs in the given
        // line segment, if values are negative or greater than 1, then the rays intersect outside the 
        // line segments we are checking
        double line1Sol = -(anchorDiff % seg2) / determinate;
        double line2Sol = seg1 % anchorDiff / determinate;

        if (line1Sol >= 0 && line1Sol <= 1 && line2Sol >= 0 && line2Sol <= 1) {
            return true;
        }
        return false;
    }
}

bool CircleCircleTest(Circle* c1, Circle* c2) {
    Vec2 centerDistance = c1->GetAnchor() - c2->GetAnchor();
    return ((centerDistance * centerDistance) < (c1->GetRadius() + c2->GetRadius()) * (c1->GetRadius() + c2->GetRadius()));
}

bool CircleLineTest(Circle* cir, Line* l) {
    Vec2 anchor1ToCenter = l->GetAnchor() - cir->GetAnchor();
    Vec2 anchor2ToCenter = l->GetAnchor2() - cir->GetAnchor();
    //if either endpoint is in circle, there is a collision
    if (anchor1ToCenter * anchor1ToCenter < cir->GetRadius() * cir->GetRadius() || anchor2ToCenter * anchor2ToCenter < cir->GetRadius() * cir->GetRadius()) {
        return true;
    }
    else {
        Vec2 directionNorm = (l->GetAnchor2() - l->GetAnchor()).Normalize();
        double b = 2 * (directionNorm * anchor1ToCenter);
        double c = (anchor1ToCenter * anchor1ToCenter) - (cir->GetRadius() * cir->GetRadius());
        //a is 1 since we normalize direction
        double tVal = cir->Quadratic(1, b, c);
        double mag = (l->GetAnchor2() - l->GetAnchor()).Magnitude();
        if (tVal >= 0 && tVal <= mag) {
            return true;
        }
        return false;
    }
}

bool CircleBoxTest(Circle* c, Box* b) {
    double boxXMin = (b->GetAnchor().x - b->GetWidth() / 2);
    double boxXMax = (b->GetAnchor().x + b->GetWidth() / 2);
    double boxYMin = (b->GetAnchor().y - b->GetHeight() / 2);
    double boxYMax = (b->GetAnchor().y + b->GetHeight() / 2);
    double closestX = c->GetAnchor().x < boxXMin ? boxXMin : c->GetAnchor().x > boxXMax ? boxXMax : c->GetAnchor().x;
    double closestY = c->GetAnchor().y < boxYMin ? boxYMin : c->GetAnchor().y > boxYMax ? boxYMax : c->GetAnchor().y;
    Vec2 vectorToClosest = Vec2(closestX, closestY) - c->GetAnchor();
    return (vectorToClosest * vectorToClosest < c->GetRadius() * c->GetRadius());
}

bool LineLineTest(Line* l1, Line* l2) {
    Vec2 seg1 = l1->GetAnchor2() - l1->GetAnchor();
    Vec2 seg2 = l2->GetAnchor2() - l2->GetAnchor();
    Vec2 anchorDiff = l1->GetAnchor() - l2->GetAnchor();
    return SegmentCollisionTest(seg1, seg2, anchorDiff);
}

bool LineBoxTest(Line* l, Box* b) {
    Vec2 seg1 = l->GetAnchor2() - l->GetAnchor();
    Vec2 blCorner = Vec2(b->GetAnchor().x - b->GetWidth() / 2, b->GetAnchor().y - b->GetHeight() / 2);
    Vec2 trCorner = Vec2(b->GetAnchor().x + b->GetWidth() / 2, b->GetAnchor().y + b->GetHeight() / 2);
    Vec2 brCorner = Vec2(b->GetAnchor().x - b->GetWidth() / 2, b->GetAnchor().y + b->GetHeight() / 2);
    Vec2 tlCorner = Vec2(b->GetAnchor().x + b->GetWidth() / 2, b->GetAnchor().y - b->GetHeight() / 2);

    //check top
    Vec2 seg2 = Vec2(-b->GetWidth(), 0);
    Vec2 anchorDiff = l->GetAnchor() - trCorner;
    if (SegmentCollisionTest(seg1, seg2, anchorDiff)) {
        return true;
    }
    //check bottom
    seg2.SetX(b->GetWidth());
    anchorDiff = l->GetAnchor() - blCorner;
    if (SegmentCollisionTest(seg1, seg2, anchorDiff)) {
        return true;
    }
    //check left
    seg2.Set(0, b->GetHeight());
    if (SegmentCollisionTest(seg1, seg2, anchorDiff)) {
        return true;
    }
    //check right
    seg2.SetY(-b->GetHeight());
    anchorDiff = l->GetAnchor() - trCorner;
    if (SegmentCollisionTest(seg1, seg2, anchorDiff)) {
        return true;
    }
    // if completely in the box, collide
    if (l->GetAnchor().x > blCorner.x && l->GetAnchor().x < trCorner.x && l->GetAnchor2().x > blCorner.x && l->GetAnchor2().x < trCorner.x && l->GetAnchor().y > blCorner.y && l->GetAnchor().x < trCorner.y && l->GetAnchor2().y > blCorner.y && l->GetAnchor2().y < trCorner.y) {
        return true;
    }
    return false;
}

bool BoxBoxTest(Box* b1, Box* b2) {
        if (abs(b1->GetAnchor().x - b2->GetAnchor().x) > (b1->GetWidth() + b2->GetWidth()) / 2) {
            return false;
        }
        if (abs(b1->GetAnchor().y - b2->GetAnchor().y) > (b1->GetHeight() + b2->GetHeight()) / 2) {
            return false;
        }
        return true;
}

void DetectionLib::DetectCollisions(vector<Circle*> circles, vector<Line*> lines, vector<Box*> boxes, vector<CollisionData>& collisionData) {
    //circle-circle, use offset to prevent duplicate evaluation
    int offset = 1;
    //circle-line
    for (int i = 0; i < circles.size(); i++) {
        Circle* c = circles[i];
        for (int j = 0; j < lines.size(); j++) {
            Line* l = lines[j];
            if (CircleLineTest(c, l)) {
                collisionData.push_back(CollisionData(c, l));
            }
        }
    }
    //circle-circle
    for (int i = 0; i < (circles.size() == 0 ? circles.size() : circles.size() - 1); i++) {
        Circle* c1 = circles[i];
        for (int j = offset; j < circles.size(); j++) {
            Circle* c2 = circles[j];
            if (CircleCircleTest(c1, c2)) {
                collisionData.push_back(CollisionData(c1, c2));
            }
        }
        offset++;
    }
    //circle-box
    for (int i = 0; i < circles.size(); i++) {
        Circle* c = circles[i];
        for (int j = 0; j < boxes.size(); j++) {
            Box* b = boxes[j];
            if (CircleBoxTest(c, b)) {
                collisionData.push_back(CollisionData(c, b));
            }
        }
    }
    //line-line, use offset to prevent duplicate evaluation
    offset = 1;
    for (int i = 0; i < (lines.size() == 0 ? lines.size() : lines.size() - 1); i++) {
        Line* l1 = lines[i];
        for (int j = offset; j < lines.size(); j++) {
            Line* l2 = lines[j];
            if (LineLineTest(l1, l2)) {
                collisionData.push_back(CollisionData(l1, l2));
            }
        }
        offset++;
    }
    //line-box
    for (int i = 0; i < lines.size(); i++) {
        Line* l = lines[i];
        for (int j = 0; j < boxes.size(); j++) {
            Box* b = boxes[j];
            if (LineBoxTest(l, b)) {
                collisionData.push_back(CollisionData(l, b));
            }
        }
    }
    //box-box, use offset to prevent duplicate evaluation
    offset = 1;
    for (int i = 0; i < (boxes.size() == 0 ? boxes.size() : boxes.size() - 1); i++) {
        Box* b1 = boxes[i];
        for (int j = offset; j < boxes.size(); j++) {
            Box* b2 = boxes[j];
            if (BoxBoxTest(b1, b2)) {
                collisionData.push_back(CollisionData(b1, b2));
            }
        }
        offset++;
    }
}

using std::chrono::high_resolution_clock;
using std::chrono::duration;
using std::chrono::milliseconds;
void DetectionLib::DebugDetectCollisions(vector<Circle*> circles, vector<Line*> lines, vector<Box*> boxes, vector<CollisionData>& collisionData, map<int, duration<double, std::milli>>& debugMap) {
    //circle-circle, use offset to prevent duplicate evaluation
    int ccCount = 0;
    int offset = 1;
    auto t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < (circles.size() == 0 ? circles.size() : circles.size() - 1); i++) {
        Circle* c1 = circles[i];
        for (int j = offset; j < circles.size(); j++) {
            Circle* c2 = circles[j];
            ccCount++;
            if (CircleCircleTest(c1, c2)) {
                collisionData.push_back(CollisionData(c1, c2));
            }
        }
        offset++;
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    duration<double, std::milli> ms_double = t2 - t1;
    duration<double, std::milli> currentTotal = debugMap[1];
    currentTotal = currentTotal + ms_double;
    debugMap[1] = currentTotal;
    cout << "circle-circle count: " << ccCount << "\n";
    //circle-line
    t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < circles.size(); i++) {
        Circle* c = circles[i];
        for (int j = 0; j < lines.size(); j++) {
            Line* l = lines[j];
            if (CircleLineTest(c, l)) {
                collisionData.push_back(CollisionData(c, l));
            }
        }
    }
    t2 = std::chrono::high_resolution_clock::now();
    ms_double = t2 - t1;
    currentTotal = debugMap[2];
    currentTotal = currentTotal + ms_double;
    debugMap[2] = currentTotal;
    //circle-box
    t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < circles.size(); i++) {
        Circle* c = circles[i];
        for (int j = 0; j < boxes.size(); j++) {
            Box* b = boxes[j];
            if (CircleBoxTest(c, b)) {
                collisionData.push_back(CollisionData(c, b));
            }
        }
    }
    t2 = std::chrono::high_resolution_clock::now();
    ms_double = t2 - t1;
    currentTotal = debugMap[3];
    currentTotal = currentTotal + ms_double;
    debugMap[3] = currentTotal;
    //line-line, use offset to prevent duplicate evaluation
    t1 = std::chrono::high_resolution_clock::now();
    int llCount = 0;
    offset = 1;
    for (int i = 0; i < (lines.size() == 0 ? lines.size() : lines.size() - 1); i++) {
        Line* l1 = lines[i];
        for (int j = offset; j < lines.size(); j++) {
            Line* l2 = lines[j];
            llCount++;
            if (LineLineTest(l1, l2)) {
                collisionData.push_back(CollisionData(l1, l2));
            }
        }
        offset++;
    }
    t2 = std::chrono::high_resolution_clock::now();
    ms_double = t2 - t1;
    currentTotal = debugMap[4];
    currentTotal = currentTotal + ms_double;
    debugMap[4] = currentTotal;
    cout << "line-line count: " << llCount << "\n";
    //line-box
    t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < lines.size(); i++) {
        Line* l = lines[i];
        for (int j = 0; j < boxes.size(); j++) {
            Box* b = boxes[j];
            if (LineBoxTest(l, b)) {
                collisionData.push_back(CollisionData(l, b));
            }
        }
    }
    t2 = std::chrono::high_resolution_clock::now();
    ms_double = t2 - t1;
    currentTotal = debugMap[5];
    currentTotal = currentTotal + ms_double;
    debugMap[5] = currentTotal;
    //box-box, use offset to prevent duplicate evaluation
    t1 = std::chrono::high_resolution_clock::now();
    int bbCount = 0;
    offset = 1;
    int count = 0;
    for (int i = 0; i < (boxes.size() == 0 ? boxes.size() : boxes.size() - 1); i++) {
        Box* b1 = boxes[i];
        for (int j = offset; j < boxes.size(); j++) {
            Box* b2 = boxes[j];
            bbCount++;
            if (BoxBoxTest(b1, b2)) {
                collisionData.push_back(CollisionData(b1, b2));
            }
        }
        offset++;
    }
    t2 = std::chrono::high_resolution_clock::now();
    ms_double = t2 - t1;
    currentTotal = debugMap[6];
    currentTotal = currentTotal + ms_double;
    debugMap[6] = currentTotal;
    cout << "box-box count: " << bbCount << "\n";
}
