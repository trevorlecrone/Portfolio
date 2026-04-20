#pragma once
#include <math.h>
#include <vector>
#include "Point.h"

struct ClusterPointsWorkerArgs {
public:
    ClusterPointsWorkerArgs(int offset_, int chunkSize_, int numPoints_, int numMedoids_, Point* medoids_, int* medoidInds_, std::vector<Point> allPoints_) : 
    offset(offset_), chunkSize(chunkSize_), numPoints(numPoints_),  numMedoids(numMedoids_), medoids(medoids_),  medoidInds(medoidInds_), allPoints(allPoints_){};
    
    ~ClusterPointsWorkerArgs() {}
public:
    int offset;
    int chunkSize;
    int numPoints;
    int numMedoids;
    Point* medoids;
    int* medoidInds; 
    std::vector<Point> allPoints;
};