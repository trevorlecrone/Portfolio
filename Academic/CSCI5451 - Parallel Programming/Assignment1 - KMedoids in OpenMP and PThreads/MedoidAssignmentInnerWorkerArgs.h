#pragma once
#include <math.h>
#include <vector>
#include "Cluster.h"

struct MedoidAssignmentInnerWorkerArgs {
public:
    MedoidAssignmentInnerWorkerArgs(int offset_, int chunkSize_, int pointsInCluster_, double &dist_, Point currentPoint_, Cluster c_) : 
    offset(offset_), chunkSize(chunkSize_), pointsInCluster(pointsInCluster_), dist(dist), currentPoint(currentPoint_), c(c_){};
    
    ~MedoidAssignmentInnerWorkerArgs() {}
public:
    int offset;
    int chunkSize;
    int pointsInCluster;
    double dist;
    Point currentPoint;
    Cluster c;
};