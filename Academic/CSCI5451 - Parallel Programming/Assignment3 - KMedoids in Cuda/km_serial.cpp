#define _POSIX_C_SOURCE 199309L
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <time.h>
#include <vector>
#include <limits>
#include "Point.h"

/**
* @brief Return the number of seconds since an unspecified time (e.g., Unix
*        epoch). This is accomplished with a high-resolution monotonic timer,
*        suitable for performance timing.
*
* @return The number of seconds.
*/
static inline double monotonic_seconds()
{
  /* Linux systems */
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec + ts.tv_nsec * 1e-9;
}

/**
* @brief Output the seconds elapsed while clustering.
*
* @param seconds Seconds spent on k-medoids clustering, excluding IO.
*/
static void print_time(double const seconds)
{
  printf("k-medoids clustering time: %0.04fs\n", seconds);
}

//Parses input file
double* ParseFile(char* fileName_, int &numPoints, int &dimension) {
    FILE *fp;
    long length;
    // need to find length to end of each line
    char line[10000];

    const char* fileName = fileName_;

    // open the file containing the scene description
    fp = fopen(fileName, "r");

    // check for errors in opening the file
    if (fp == NULL) {
        printf("Can't open file '%s'\n", fileName);
        double* fail = new double[1];
        fail[0] = -1.0f;
        return fail;  //Exit
    }
    
    //get info from first line
    fgets(line, 10000, fp);
    sscanf(line, "%d %d", &numPoints, &dimension);
    double* vals = new double[numPoints * dimension];

    int currentPosition = 0;
    while (currentPosition < numPoints) {
        fgets(line, 10000, fp);
        char* target = strtok(line, " ");;
        for(int i = 0; i < dimension; i++) {
            sscanf(target, "%lf", &vals[(currentPosition * dimension) + i]);
            target = strtok(NULL, " ");
        }
        currentPosition++;
    }
    fclose(fp);
    return vals;
}


bool clusterPoints(int numMedoids, int numPoints, Point* medoids, Point* allPoints, int* &medoidInds, int* &pointsInClusters) {
    bool convergence = true;
    for (int i = 0; i < numMedoids; i++) {
        pointsInClusters[i] = 0;
    }

    for (int i = 0; i < numPoints; i++) {
        Point p = allPoints[i];
        double minDistance = std::numeric_limits<double>::max();
        int minDistInd = 0;
        for (int j = 0; j < numMedoids; j++) {
            double distance = p.distanceTo(medoids[j]);
            if (distance <= minDistance) {
                minDistance = distance;
                minDistInd = j;
            }
        }
        pointsInClusters[minDistInd]++;
        if(medoidInds[i] != minDistInd) {
            medoidInds[i] = minDistInd;
            convergence = false;
        }
    }
    return convergence;
}

Point* getClusterPoints(Point* allPoints, int numPoints, int clusterId, int* medoidInds, int pointsInCluster) {
    Point* clusterPoints = new Point[pointsInCluster];
    int clusterPointsIndex = 0;
    for(int i = 0; i < numPoints; i++) {
        if(medoidInds[i] == clusterId) {
            clusterPoints[clusterPointsIndex] = allPoints[i];
            clusterPointsIndex++;
        }
    }
    return clusterPoints;
}

void assignMedoid(Point* clusterPoints, Point* medoids, int clusterId, int pointsInCluster) {
    double* dists = new double[pointsInCluster];
    for (int p = 0; p < pointsInCluster; p++) {
        double totalDist = 0.0;
        Point currentPoint = clusterPoints[p];
        for (int p2 = 0; p2 < pointsInCluster; p2++) {
            Point secondPoint =clusterPoints[p2];
            totalDist += currentPoint.distanceTo(secondPoint);
        }
        dists[p] = totalDist;
    }
    double minDist = dists[0];
    int minDistInd = 0;
    for (int i = 1; i < pointsInCluster; i++) {
        if(dists[i] < minDist) {
            minDist = dists[i];
            minDistInd = i;
        }
    }
    medoids[clusterId] = clusterPoints[minDistInd];
    
}

int main(int argc, char** argv){

	// parse arguments
    char* fileName = argv[1];
    int numGroups = atoi(argv[2]);
    int numThreads = atoi(argv[3]);

    int numPoints;
    int dimension;

    double* allCoordComponents = ParseFile(fileName, numPoints, dimension);

    int numDoubles = numPoints * dimension;

    std::vector<Point> allPoints;

    for(int i = 0; i < numPoints; i++) {
        double* pointCoords = new double[dimension];
        memcpy(pointCoords, &allCoordComponents[i * dimension],  dimension * sizeof(double));
        allPoints.push_back(*(new Point(dimension, pointCoords)));
    }
    Point* pointsAsArray = allPoints.data();
    allPoints.clear();
    Point* cudaPoints = new Point[numPoints];
    delete[] allCoordComponents;

    double start = monotonic_seconds();
    int* medoidInds = new int[numPoints]();
    int* pointsInClusters = new int[numGroups]();
    Point medoids[numGroups];
    for (int i = 0; i < numGroups; i++) {
        medoids[i] = allPoints[i];
        pointsInClusters[i] = 0;
    }
    double start_iter = monotonic_seconds();
    clusterPoints(numGroups, numPoints, medoids, pointsAsArray, medoidInds, pointsInClusters);
    std::ofstream PICFile;
    /* PICFile.open("PIC.txt");
    int sum = 0;
    for (int i = 0; i < numGroups; i++) {
        PICFile << pointsInClusters[i] << "\n";
        sum += pointsInClusters[i];
    }
    PICFile << "sum " << sum << "\n";
    PICFile.close(); */
    double stop_iter = monotonic_seconds();
    bool converged = false;
    //max iterations = 20
    for (int i = 0; i < 20; i++) {
        converged = false;
        double start_medoid = monotonic_seconds();
        for (int j = 0; j < numGroups; j++) {
            Point* clusterPoints = getClusterPoints(pointsAsArray, numPoints, j, medoidInds, pointsInClusters[j]);
            assignMedoid(clusterPoints, medoids, j, pointsInClusters[j]);
        }
        double stop_medoid = monotonic_seconds();
        double start_iter = monotonic_seconds();
        clusterPoints(numGroups, numPoints, medoids, pointsAsArray, medoidInds, pointsInClusters);
        double stop_iter = monotonic_seconds();
        printf("medoid calculation time: %0.04fs\n", stop_medoid - start_medoid);
        printf("iteration clustering time: %0.04fs\n", stop_iter - start_iter);
        if(converged) {
            i += 20;
        }
    }
    double stop = monotonic_seconds();
    double clusteringTime = stop - start;
    print_time(clusteringTime);

    std::ofstream clustersFile;
    clustersFile.open("clusters.txt");
    for (int i = 0; i < numPoints; i++) {
        clustersFile << medoidInds[i] << "\n";
    }
    clustersFile.close();

    std::ofstream medoidFile;
    medoidFile.open("medoids.txt");
    medoidFile << std::fixed << std::setprecision(8);
    for (int i = 0; i < numGroups; i++) {
        for (int j = 0; j < dimension - 1; j++) {
            double val = medoids[i].coordinates[j];
            if(val > 1.0 || val < -1.0) {
                medoidFile  << std::fixed << std::setprecision(8) << val << " ";
            }
            else {
                medoidFile.unsetf(std::ios_base::fixed);
                medoidFile << val << " ";
            }
        }
        double fVal = medoids[i].coordinates[dimension - 1];
        if(fVal > 1.0 || fVal < -1.0) {
            medoidFile  << std::fixed << std::setprecision(8) << fVal << "\n";
        }
        else {
            medoidFile.unsetf(std::ios_base::fixed);
            medoidFile << fVal << "\n";
        }
    }
    medoidFile.close();
}