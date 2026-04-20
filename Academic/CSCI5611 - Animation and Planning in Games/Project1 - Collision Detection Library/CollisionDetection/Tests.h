#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <set>
#include <vector>
#include <chrono>
#include "Circle.h"
#include "Box.h"
#include "Line.h"
#include "DetectionLib.h"
static void runTests() {

    cout << "running tests... \n";
    vector<Circle*> circles;
    vector<Line*> lines;
    vector<Box*> boxes;
    vector<CollisionData> collisions = {};

    //Line-Line cases, will be most robust since most of line-box depends on this. Will also have some line box tests to ensure logic
    //for breaking up those collisions into line-line is correct

    //prevent need to hard code number of tests
    int totalTests = 0;
    int passes = 0;

    cout << "LINE-LINE TESTS\n";
    cout << "--------------------------------------------------\n";
    //parallel, vertically aligned
    totalTests++;
    lines.push_back(new Line(Vec2(0.0, 1.0), Vec2(0.0, 2.0), 0));
    lines.push_back(new Line(Vec2(1.0, 1.0), Vec2(1.0, 2.0), 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    bool testResult = collisions.size() == 0;
    if (testResult) passes++;
    cout << "Line-Line, parallel vertical does not return collision: " << (testResult ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "\n";
    lines.clear();
    collisions.clear();

    //parallel, horizontally aligned
    totalTests++;
    lines.push_back(new Line(Vec2(0.0, 0.0), Vec2(1.0, 0.0), 0));
    lines.push_back(new Line(Vec2(0.0, 1.0), Vec2(1.0, 1.0), 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    testResult = collisions.size() == 0;
    if (testResult) passes++;
    cout << "Line-Line, parallel horizontal does not return collision: " << (testResult ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "\n";
    lines.clear();
    collisions.clear();

    //parallel, diagonally aligned
    totalTests++;
    lines.push_back(new Line(Vec2(0.0, 0.0), Vec2(1.0, 1.0), 0));
    lines.push_back(new Line(Vec2(0.0, 1.0), Vec2(1.0, 2.0), 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    testResult = collisions.size() == 0;
    if (testResult) passes++;
    cout << "Line-Line, parallel diagonal does not return collision: " << (testResult ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "\n";
    lines.clear();
    collisions.clear();

    //Ray intersects, segments do not, axis aligned
    totalTests++;
    lines.push_back(new Line(Vec2(0.0, 0.0), Vec2(1.0, 0.0), 0));
    lines.push_back(new Line(Vec2(0.5, 0.5), Vec2(0.5, 0.1), 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    testResult = collisions.size() == 0;
    if (testResult) passes++;
    cout << "Line-Line, ray intersects, segments do not axis aligned does not return collision: " << (testResult ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "\n";
    lines.clear();
    collisions.clear();

    //Ray intersects, segments do not, diagonal
    totalTests++;
    lines.push_back(new Line(Vec2(0.0, 0.0), Vec2(1.0, 1.0), 0));
    lines.push_back(new Line(Vec2(0.5, 0.45), Vec2(-0.5, -0.55), 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    testResult = collisions.size() == 0;
    if (testResult) passes++;
    cout << "Line-Line, ray intersects, segments do not diagonal does not return collision: " << (testResult ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "\n";
    lines.clear();
    collisions.clear();

    //mid-point - mid-point intersection axis aligned
    totalTests++;
    lines.push_back(new Line(Vec2(0.0, 0.0), Vec2(1.0, 0.0), 0));
    lines.push_back(new Line(Vec2(0.5, 0.5), Vec2(0.5, -0.5), 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    testResult = collisions.size() == 1;
    if (testResult) passes++;
    cout << "Line-Line, mid-point - mid-point intersection axis aligned return collision: " << (testResult ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "\n";
    lines.clear();
    collisions.clear();

    //mid-point - mid-point intersection diagonal
    totalTests++;
    lines.push_back(new Line(Vec2(0.0, 0.0), Vec2(1.0, 1.0), 0));
    lines.push_back(new Line(Vec2(0.0, 1.0), Vec2(1.0, 0.0), 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    testResult = collisions.size() == 1;
    if (testResult) passes++;
    cout << "Line-Line, mid-point - mid-point intersection diagonal return collision: " << (testResult ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "\n";
    lines.clear();
    collisions.clear();

    //mid-point - end-point intersection axis aligned
    totalTests++;
    lines.push_back(new Line(Vec2(0.0, 0.0), Vec2(0.0, 1.0), 0));
    lines.push_back(new Line(Vec2(-0.5, 1.0), Vec2(0.5, 1.0), 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    testResult = collisions.size() == 1;
    if (testResult) passes++;
    cout << "Line-Line, mid-point - end-point intersection axis aligned return collision: " << (testResult ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "\n";
    lines.clear();
    collisions.clear();

    //mid-point - end-point intersection diagonal
    totalTests++;
    lines.push_back(new Line(Vec2(0.0, 0.0), Vec2(1.0, 1.0), 0));
    lines.push_back(new Line(Vec2(0.75, 0.75), Vec2(1.75, -0.25), 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    testResult = collisions.size() == 1;
    if (testResult) passes++;
    cout << "Line-Line, mid-point - end-point intersection diagonal return collision: " << (testResult ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "\n";
    lines.clear();
    collisions.clear();

    //end-point - end-point intersection axis aligned
    totalTests++;
    lines.push_back(new Line(Vec2(0.0, 0.0), Vec2(0.0, 1.0), 0));
    lines.push_back(new Line(Vec2(0.0, 1.0), Vec2(1.0, 1.0), 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    testResult = collisions.size() == 1;
    if (testResult) passes++;
    cout << "Line-Line, end-point - end-point intersection axis aligned return collision: " << (testResult ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "\n";
    lines.clear();
    collisions.clear();

    //end-point - end-point intersection diagonal
    totalTests++;
    lines.push_back(new Line(Vec2(0.0, 0.0), Vec2(1.0, 1.0), 0));
    lines.push_back(new Line(Vec2(1.0, 1.0), Vec2(2.0, 0.0), 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    testResult = collisions.size() == 1;
    if (testResult) passes++;
    cout << "Line-Line, end-point - end-point intersection diagonal return collision: " << (testResult ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "\n";
    lines.clear();
    collisions.clear();

    //end-point - end-point intersection collinear
    totalTests++;
    lines.push_back(new Line(Vec2(0.0, 0.0), Vec2(1.0, 1.0), 0));
    lines.push_back(new Line(Vec2(1.0, 1.0), Vec2(2.0, 2.0), 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    testResult = collisions.size() == 1;
    if (testResult) passes++;
    cout << "Line-Line, end-point - end-point intersection collinear return collision: " << (testResult ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "\n";
    lines.clear();
    collisions.clear();

    //mid-point - end-point intersection collinear
    totalTests++;
    lines.push_back(new Line(Vec2(0.0, 0.0), Vec2(1.0, 1.0), 0));
    lines.push_back(new Line(Vec2(0.5, 0.5), Vec2(1.5, 1.5), 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    testResult = collisions.size() == 1;
    if (testResult) passes++;
    cout << "Line-Line, mid-point - end-point intersection collinear return collision: " << (testResult ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "\n";
    lines.clear();
    collisions.clear();

    //fully collinear segments
    totalTests++;
    lines.push_back(new Line(Vec2(0.0, 0.0), Vec2(1.0, 1.0), 0));
    lines.push_back(new Line(Vec2(0.0, 0.0), Vec2(1.0, 1.0), 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    testResult = collisions.size() == 1;
    if (testResult) passes++;
    cout << "Line-Line, fully collinear segments return collision: " << (testResult ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "\n";
    lines.clear();
    collisions.clear();

    cout << "--------------------------------------------------\n";
    cout << "LINE-CRICLE TESTS\n";
    cout << "--------------------------------------------------\n";
    //Line fully outside circle, axis aligned
    totalTests++;
    lines.push_back(new Line(Vec2(1.1, 0.0), Vec2(2.0, 0.0), 0));
    circles.push_back(new Circle(Vec2(0.0, 0.0), 1.0, 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    testResult = collisions.size() == 0;
    if (testResult) passes++;
    cout << "Line-Circle, segment fully outside circle, axis aligned: " << (testResult ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "\n";
    lines.clear();
    circles.clear();
    collisions.clear();

    //Line fully outside circle, diagonal
    totalTests++;
    lines.push_back(new Line(Vec2(0.9, 0.9), Vec2(1.9, 0.9), 0));
    circles.push_back(new Circle(Vec2(0.0, 0.0), 1.0, 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    testResult = collisions.size() == 0;
    if (testResult) passes++;
    cout << "Line-Circle, segment fully outside circle, diagonal: " << (testResult ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "\n";
    lines.clear();
    circles.clear();
    collisions.clear();

    //Line fully inside circle
    totalTests++;
    lines.push_back(new Line(Vec2(0.0, 0.0), Vec2(0.0, 0.5), 0));
    circles.push_back(new Circle(Vec2(0.0, 0.0), 2.0, 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    testResult = collisions.size() == 1;
    if (testResult) passes++;
    cout << "Line-Circle, segment fully inside circle: " << (testResult ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "\n";
    lines.clear();
    circles.clear();
    collisions.clear();

    //Line intersects with 1 endpoint within circle
    totalTests++;
    lines.push_back(new Line(Vec2(1.0, 0.0), Vec2(5.0, 0.0), 0));
    circles.push_back(new Circle(Vec2(0.0, 0.0), 2.0, 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    testResult = collisions.size() == 1;
    if (testResult) passes++;
    cout << "Line-Circle, segment has 1 endpoint inside circle: " << (testResult ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "\n";
    lines.clear();
    circles.clear();
    collisions.clear();

    //Line intersects with endpoints outside circle
    totalTests++;
    lines.push_back(new Line(Vec2(-3.0, 0.0), Vec2(3.0, 0.0), 0));
    circles.push_back(new Circle(Vec2(0.0, 0.0), 2.0, 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    testResult = collisions.size() == 1;
    if (testResult) passes++;
    cout << "Line-Circle, segment intersects and has endpoints outside circle: " << (testResult ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "\n";
    lines.clear();
    circles.clear();
    collisions.clear();

    //Line has endpoint tangent to circle
    totalTests++;
    lines.push_back(new Line(Vec2(2.0, 0.0), Vec2(3.0, 0.0), 0));
    circles.push_back(new Circle(Vec2(0.0, 0.0), 2.0, 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    testResult = collisions.size() == 1;
    if (testResult) passes++;
    cout << "Line-Circle, segment has endpoint tangent to circle: " << (testResult ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "\n";
    lines.clear();
    circles.clear();
    collisions.clear();

    cout << "--------------------------------------------------\n";
    cout << "LINE-BOX TESTS\n";
    cout << "--------------------------------------------------\n";
    //Line fully outside box
    totalTests++;
    lines.push_back(new Line(Vec2(0.0, 1.1), Vec2(0.0, 2.1), 0));
    boxes.push_back(new Box(Vec2(0.0, 0.0), 2.0, 2.0, 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    testResult = collisions.size() == 0;
    if (testResult) passes++;
    cout << "Line-Box, segment fully outside box: " << (testResult ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "\n";
    lines.clear();
    boxes.clear();
    collisions.clear();

    //Line fully inside box
    totalTests++;
    lines.push_back(new Line(Vec2(-0.5, 0.5), Vec2(0.5, 0.5), 0));
    boxes.push_back(new Box(Vec2(0.0, 0.0), 2.0, 2.0, 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    testResult = collisions.size() == 1;
    if (testResult) passes++;
    cout << "Line-Box, segment fully inside box: " << (testResult ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "\n";
    lines.clear();
    boxes.clear();
    collisions.clear();

    //Line intersects top of box
    totalTests++;
    lines.push_back(new Line(Vec2(0, 1.5), Vec2(0, 0.5), 0));
    boxes.push_back(new Box(Vec2(0.0, 0.0), 2.0, 2.0, 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    testResult = collisions.size() == 1;
    if (testResult) passes++;
    cout << "Line-Box, segment intersects top of box: " << (testResult ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "\n";
    lines.clear();
    boxes.clear();
    collisions.clear();

    //Line intersects bottom of box
    totalTests++;
    lines.push_back(new Line(Vec2(0, -1.5), Vec2(0, -0.5), 0));
    boxes.push_back(new Box(Vec2(0.0, 0.0), 2.0, 2.0, 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    testResult = collisions.size() == 1;
    if (testResult) passes++;
    cout << "Line-Box, segment intersects bottom of box: " << (testResult ? "\033[32mPASS\033[0m" : "\033[33m\033[31mFAIL\033[0m\033[0m") << "\n";
    lines.clear();
    boxes.clear();
    collisions.clear();

    //Line intersects right of box
    totalTests++;
    lines.push_back(new Line(Vec2(1.5, 0.0), Vec2(0.5, 0.0), 0));
    boxes.push_back(new Box(Vec2(0.0, 0.0), 2.0, 2.0, 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    testResult = collisions.size() == 1;
    if (testResult) passes++;
    cout << "Line-Box, segment intersects right of box: " << (testResult ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "\n";
    lines.clear();
    boxes.clear();
    collisions.clear();

    //Line intersects left of box
    totalTests++;
    lines.push_back(new Line(Vec2(-1.5, 0.0), Vec2(-0.5, 0.0), 0));
    boxes.push_back(new Box(Vec2(0.0, 0.0), 2.0, 2.0, 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    testResult = collisions.size() == 1;
    if (testResult) passes++;
    cout << "Line-Box, segment intersects left of box: " << (testResult ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "\n";
    lines.clear();
    boxes.clear();
    collisions.clear();

    //Line intersects corner of box
    totalTests++;
    lines.push_back(new Line(Vec2(0.0, 2.0), Vec2(2.0, 0.0), 0));
    boxes.push_back(new Box(Vec2(0.0, 0.0), 2.0, 2.0, 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    testResult = collisions.size() == 1;
    if (testResult) passes++;
    cout << "Line-Box, segment intersects corner of box: " << (testResult ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "\n";
    lines.clear();
    boxes.clear();
    collisions.clear();

    //Line anchored to corner
    totalTests++;
    lines.push_back(new Line(Vec2(1.0, 1.0), Vec2(2.0, 2.0), 0));
    boxes.push_back(new Box(Vec2(0.0, 0.0), 2.0, 2.0, 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    testResult = collisions.size() == 1;
    if (testResult) passes++;
    cout << "Line-Box, segment anchored to corner of box: " << (testResult ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "\n";
    lines.clear();
    boxes.clear();
    collisions.clear();

    cout << "--------------------------------------------------\n";
    cout << "CIRCLE-BOX TESTS\n";
    cout << "--------------------------------------------------\n";
    //Circle and box do not overlap
    totalTests++;
    circles.push_back(new Circle(Vec2(0.0, 3.0), 1.0, 0));
    boxes.push_back(new Box(Vec2(0.0, 0.0), 2.0, 2.0, 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    testResult = collisions.size() == 0;
    if (testResult) passes++;
    cout << "Circle-Box, circle fully outside box: " << (testResult ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "\n";
    circles.clear();
    boxes.clear();
    collisions.clear();

    //Circle center inside box
    totalTests++;
    circles.push_back(new Circle(Vec2(0.5, 0.5), 1.0, 0));
    boxes.push_back(new Box(Vec2(0.0, 0.0), 2.0, 2.0, 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    testResult = collisions.size() == 1;
    if (testResult) passes++;
    cout << "Circle-Box, circle center fully inside box: " << (testResult ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "\n";
    circles.clear();
    boxes.clear();
    collisions.clear();

    //circle intersects top of box
    totalTests++;
    circles.push_back(new Circle(Vec2(0.0, 1.5), 0.75, 0));
    boxes.push_back(new Box(Vec2(0.0, 0.0), 2.0, 2.0, 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    testResult = collisions.size() == 1;
    if (testResult) passes++;
    cout << "Circle-Box, circle intersects top of box: " << (testResult ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "\n";
    circles.clear();
    boxes.clear();
    collisions.clear();

    //circle intersects bottom of box
    totalTests++;
    circles.push_back(new Circle(Vec2(0.0, -1.5), 0.75, 0));
    boxes.push_back(new Box(Vec2(0.0, 0.0), 2.0, 2.0, 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    testResult = collisions.size() == 1;
    if (testResult) passes++;
    cout << "Circle-Box, circle intersects bottom of box: " << (testResult ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "\n";
    circles.clear();
    boxes.clear();
    collisions.clear();

    //circle intersects left of box
    totalTests++;
    circles.push_back(new Circle(Vec2(-1.5, 0.0), 0.75, 0));
    boxes.push_back(new Box(Vec2(0.0, 0.0), 2.0, 2.0, 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    testResult = collisions.size() == 1;
    if (testResult) passes++;
    cout << "Circle-Box, circle intersects left of box: " << (testResult ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "\n";
    circles.clear();
    boxes.clear();
    collisions.clear();

    //circle intersects right of box
    totalTests++;
    circles.push_back(new Circle(Vec2(1.5, 0.0), 0.75, 0));
    boxes.push_back(new Box(Vec2(0.0, 0.0), 2.0, 2.0, 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    testResult = collisions.size() == 1;
    if (testResult) passes++;
    cout << "Circle-Box, circle intersects right of box: " << (testResult ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "\n";
    circles.clear();
    boxes.clear();
    collisions.clear();

    //circle and box corner are tangent
    totalTests++;
    circles.push_back(new Circle(Vec2(0.0, 0.0), 1.0, 0));
    boxes.push_back(new Box(Vec2(2.0, 1.0), 2.0, 2.0, 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    testResult = collisions.size() == 1;
    if (testResult) passes++;
    cout << "Circle-Box, circle and box corner are tangent: " << (testResult ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "\n";
    circles.clear();
    boxes.clear();
    collisions.clear();

    cout << "--------------------------------------------------\n";
    cout << "CIRCLE-CIRCLE TESTS\n";
    cout << "--------------------------------------------------\n";
    //Circles do not overlap
    totalTests++;
    circles.push_back(new Circle(Vec2(0.0, 3.0), 1.0, 0));
    circles.push_back(new Circle(Vec2(0.0, 0.0), 1.0, 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    testResult = collisions.size() == 0;
    if (testResult) passes++;
    cout << "Circle-Circle, circles do not overlap: " << (testResult ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "\n";
    circles.clear();
    collisions.clear();

    //Circles overlap, centers axis aligned
    totalTests++;
    circles.push_back(new Circle(Vec2(0.0, 3.0), 1.0, 0));
    circles.push_back(new Circle(Vec2(0.0, 2.5), 1.0, 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    testResult = collisions.size() == 1;
    if (testResult) passes++;
    cout << "Circle-Circle, circles overlap, centers axis aligned: " << (testResult ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "\n";
    circles.clear();
    collisions.clear();

    //Circles tangent
    totalTests++;
    circles.push_back(new Circle(Vec2(0.0, 3.0), 1.0, 0));
    circles.push_back(new Circle(Vec2(0.0, 2.0), 1.0, 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    testResult = collisions.size() == 1;
    if (testResult) passes++;
    cout << "Circle-Circle, circles are tangential: " << (testResult ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "\n";
    circles.clear();
    collisions.clear();

    //Circles overlap centers diagonal
    totalTests++;
    circles.push_back(new Circle(Vec2(0.0, 3.0), 1.0, 0));
    circles.push_back(new Circle(Vec2(0.9, 4.2), 1.0, 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    testResult = collisions.size() == 1;
    if (testResult) passes++;
    cout << "Circle-Circle, circles overlap, centers diagonal: " << (testResult ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "\n";
    circles.clear();
    collisions.clear();

    cout << "--------------------------------------------------\n";
    cout << "BOX-BOX TESTS\n";
    cout << "--------------------------------------------------\n";
    //Boxes do not overlap
    totalTests++;
    boxes.push_back(new Box(Vec2(0.0, 0.0), 2.0, 2.0, 0));
    boxes.push_back(new Box(Vec2(0.0, 4.0), 2.0, 2.0, 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    testResult = collisions.size() == 0;
    if (testResult) passes++;
    cout << "Box-Box, boxes do not overlap: " << (testResult ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "\n";
    boxes.clear();
    collisions.clear();

    //Boxes share corner
    totalTests++;
    boxes.push_back(new Box(Vec2(0.0, 0.0), 2.0, 2.0, 0));
    boxes.push_back(new Box(Vec2(2.0, 2.0), 2.0, 2.0, 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    testResult = collisions.size() == 1;
    if (testResult) passes++;
    cout << "Box-Box, boxes share corner: " << (testResult ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "\n";
    boxes.clear();
    collisions.clear();

    //Boxes overlap
    totalTests++;
    boxes.push_back(new Box(Vec2(0.0, 0.0), 2.0, 2.0, 0));
    boxes.push_back(new Box(Vec2(1.6, 1.6), 2.0, 2.0, 1));
    DetectionLib::DetectCollisions(circles, lines, boxes, collisions);
    testResult = collisions.size() == 1;
    if (testResult) passes++;
    cout << "Box-Box, boxes overlap: " << (testResult ? "\033[32mPASS\033[0m" : "\033[31mFAIL\033[0m") << "\n";
    boxes.clear();
    collisions.clear();

    cout << "Tests finished, Passed " << passes << " of " << totalTests <<" tests\n";
    return;
}