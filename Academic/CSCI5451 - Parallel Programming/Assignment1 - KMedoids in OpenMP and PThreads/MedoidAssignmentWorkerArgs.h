#pragma once
#include <math.h>
#include <vector>
#include "Cluster.h"

struct MedoidAssignmentWorkerArgs {
public:
    MedoidAssignmentWorkerArgs(int offset_, int chunkSize_, int pointsInCluster_, double* &dists_, Cluster c_) : 
    offset(offset_), chunkSize(chunkSize_), pointsInCluster(pointsInCluster_), dists(dists_),  c(c_){};
    
    ~MedoidAssignmentWorkerArgs() {}
public:
    int offset;
    int chunkSize;
    int pointsInCluster;
    double* dists;
    Cluster c;
};