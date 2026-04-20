// CollisionDetection.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

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
#include "Tests.h"
#include "CollisionData.h"
using namespace std;
const int LINE_MAX = 1024;
const int CIRCLES = 1;
const int LINES = 2;
const int BOXES = 3;

#define DEBUG_MODE false

#define TEST_MODE false

void parse(char* filename_, vector<Circle*>& circles, vector<Line*>& lines, vector<Box*>& boxes) {

    printf("parsing input file...\n");
    FILE* fp;
    long length;
    char line[LINE_MAX];

    string fileName = filename_;

    // open the file
    fp = fopen(fileName.c_str(), "r");

    // check for errors in opening the file
    if (fp == NULL) {
        printf("Can't open file, '%s', ensure it is present and passed as the first argument after the executable name\n", fileName.c_str());
        return;  //Exit
    }
    int currentPrimitive = 0;
    //Loop through reading each line
    while (fgets(line, LINE_MAX, fp)) {
        if (line[0] == '#') {
            //printf("Skipping comment: %s\n", line);
            continue;
        }

        char command[100];
        int fieldsRead = sscanf(line, "%s ", command); //Read first word in the line (i.e., the command type)
        string typeStr = command;

        if (fieldsRead < 1) { //No command read
            //Blank line
            continue;
        }
        int numCircles, numLines, numBoxes;
        int id;
        if (typeStr == "Circles:") {
            currentPrimitive = CIRCLES;
            sscanf(line, "Circles: %d", &numCircles);
            printf("Number of Circles: %d\n", numCircles);
        }
        else if (typeStr == "Lines:") {
            currentPrimitive = LINES;
            sscanf(line, "Lines: %d", &numLines);
            printf("Number of Lines: %d\n", numLines);
        }
        else if (typeStr == "Boxes:") {
            currentPrimitive = BOXES;
            sscanf(line, "Boxes: %d", &numBoxes);
            printf("Number of Boxes: %d\n", numBoxes);
        }
        else if (currentPrimitive == CIRCLES) {
            double center_x, center_y, radius;
            sscanf(line, "%d : %lf %lf %lf", &id, &center_x, &center_y, &radius);
            circles.push_back(new Circle(Vec2(center_x, center_y), radius, id));
            //printf("Circle %d: (%lf, %lf), %lf\n", id, center_x, center_y, radius);
        }
        else if (currentPrimitive == LINES) {
            double x1, y1, x2, y2;
            sscanf(line, "%d : %lf %lf %lf %lf", &id, &x1, &y1, &x2, &y2);
            lines.push_back(new Line(Vec2(x1, y1), Vec2(x2, y2), id));
            //printf("Line %d: (%lf, %lf), (%lf, %lf)\n", id, x1, y1, x2, y2);
        }
        else if (currentPrimitive == BOXES) {
            double center_x, center_y, height, width;
            sscanf(line, "%d : %lf %lf %lf %lf", &id, &center_x, &center_y, &width, &height);
            boxes.push_back(new Box(Vec2(center_x, center_y), width, height, id));
            //printf("Box %d: (%lf, %lf), width: %lf, height: %lf)\n", id, center_x, center_y, width, height);
        }
    }
    printf("parsing complete\n");
    fclose(fp);
    return;
}

int main(int argc, char** argv)
{
    using std::chrono::high_resolution_clock;
    using std::chrono::duration;
    using std::chrono::milliseconds;

    if (TEST_MODE) {
        runTests();
    }

    char* fileName;
    if (!DEBUG_MODE) {
        if (argc < 2) {
            cout << "Usage: ./CollisionDetection <task file name>\n";
            exit(0);
        }
        fileName = argv[1];
    }
    else {
        fileName = (char*)"task10.txt";
    }

    vector<Circle*> circles;
    vector<Line*> lines;
    vector<Box*> boxes;
    parse(fileName, circles, lines, boxes);
    vector<CollisionData> collidingPrimitives = {};

    
    map<int, duration<double, std::milli>> debugMap;
    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = std::chrono::high_resolution_clock::now();
    if (DEBUG_MODE) {
        map<int, duration<double, std::milli>> debugMap;
        debugMap[1] = (duration<double, std::milli>) 0.0;
        debugMap[2] = (duration<double, std::milli>) 0.0;
        debugMap[3] = (duration<double, std::milli>) 0.0;
        debugMap[4] = (duration<double, std::milli>) 0.0;
        debugMap[5] = (duration<double, std::milli>) 0.0;
        debugMap[6] = (duration<double, std::milli>) 0.0;
        cout << "Circles: " << circles.size() << "\n";
        cout << "Lines: " << lines.size() << "\n";
        cout << "Boxes: " << boxes.size() << "\n";
        t1 = std::chrono::high_resolution_clock::now();
        DetectionLib::DebugDetectCollisions(circles, lines, boxes, collidingPrimitives, debugMap);
        t2 = std::chrono::high_resolution_clock::now();

        cout << "Circle-Circle: " << debugMap[1].count() << "\n";
        cout << "Circle-Line: " << debugMap[2].count() << "\n";
        cout << "Circle-Box: " << debugMap[3].count() << "\n";
        cout << "Line-Box: " << debugMap[4].count() << "\n";
        cout << "Line-Line: " << debugMap[5].count() << "\n";
        cout << "Box-Box: " << debugMap[6].count() << "\n";
        
    }
    else {
        t1 = std::chrono::high_resolution_clock::now();
        DetectionLib::DetectCollisions(circles, lines, boxes, collidingPrimitives);
        t2 = std::chrono::high_resolution_clock::now();
    }
    duration<double, std::milli> ms_double = t2 - t1;
    //Not including processing of collisionData into ID list in time
    set<int> collidingPrimitiveIds = {};
    for (CollisionData collision : collidingPrimitives) {
        collidingPrimitiveIds.insert(collision.p1.GetID());
        collidingPrimitiveIds.insert(collision.p2.GetID());
    }
    string fnameString = (string)fileName;
    int prefixLength = fnameString.length() - 4;
    char* outFileName = (char*)((fnameString.substr(0, prefixLength) + "_solution.txt").c_str());
    ofstream outFile;
    outFile.open(outFileName);
    outFile << "Duration: " << ms_double.count() << "ms\n";
    outFile << "Num Collisions: " << collidingPrimitiveIds.size() << "\n";
    for (int id : collidingPrimitiveIds) {
        outFile << id << "\n";
    }
    outFile.close();
}