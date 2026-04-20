#pragma once
#include <math.h>

struct Point {
public:
    Point() :  dimensions(2){
        coordinates = new double[2]();
        coordinates[0] = 1.0;
        coordinates[1] = 1.0;
    };
    Point(int dimensions_, double* coordinates_) : dimensions(dimensions_), coordinates(coordinates_){};

    double distanceTo(Point p) {
        double sum = 0.0;
        for (int i = 0; i < dimensions; i++) {
            double distComp = this->coordinates[i] - p.coordinates[i];
            sum += (distComp * distComp);
        }
        return sum;
    }
    
    ~Point() {}
public:
    int dimensions;
    double* coordinates;
};