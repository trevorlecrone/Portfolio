#pragma once
#include <math.h>
#include <vector>
#include "Point.h"

struct Cluster {
public:
    Cluster() :  medoid(*new Point()){
        allPoints.push_back(medoid);
    };
    Cluster(Point medoid_, std::vector<Point> allPoints_) : medoid(medoid_), allPoints(allPoints_){};
    
    ~Cluster() {}
public:
    Point medoid;
    std::vector<Point> allPoints;
};