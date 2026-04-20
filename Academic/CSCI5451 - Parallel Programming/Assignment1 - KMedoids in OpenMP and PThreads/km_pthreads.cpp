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
#include <pthread.h>
#include "Cluster.h"
#include "ClusterPointsWorkerArgs.h"
#include "MedoidAssignmentWorkerArgs.h"

int N_THREADS = 0;
pthread_mutex_t medoidLock = PTHREAD_MUTEX_INITIALIZER;

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

// ----------------------------
// Thread Workers
// ----------------------------
void* clusterPointsWorker(void * args_) {
    double minDistance = std::numeric_limits<double>::max();
    int minDistInd;
    ClusterPointsWorkerArgs* args =  (ClusterPointsWorkerArgs*) args_;
    
    for (int i = args->offset; i < args->numPoints; i += args->chunkSize) {
        Point p = args->allPoints[i];
        minDistance = std::numeric_limits<double>::max();
        minDistInd = 0;
        for (int j = 0; j < args->numMedoids; j++) {
            double distance = p.distanceTo(args->medoids[j]);
            if (distance <= minDistance) {
                minDistance = distance;
                minDistInd = j;
            }
        }
        args->medoidInds[i] = minDistInd;
    }
    pthread_exit(NULL);
    return 0;
}

void* medoidAssingmentWorker(void * args_) {
    MedoidAssignmentWorkerArgs* args =  (MedoidAssignmentWorkerArgs*) args_;
    
    for (int p = args->offset; p < args->pointsInCluster; p += args->chunkSize) {
        double totalDist = 0.0;
        Point currentPoint = args->c.allPoints[p];
        for (int p2 = 0; p2 < args->pointsInCluster; p2++) {
            Point secondPoint = args->c.allPoints[p2];
            totalDist += currentPoint.distanceTo(secondPoint);
        }
        args->dists[p] = totalDist;
    }
    pthread_exit(NULL);
}

//-----------------------------
// Clustering functions
//-----------------------------

std::vector<Cluster> clusterPointsInitial(int numMedoids, int numPoints, Point* medoids, std::vector<Point> allPoints, int* &medoidInds) {
    std::vector<Cluster> clusters;
    for(int i = 0; i < numMedoids; i++) {
        clusters.push_back(*(new Cluster()));
        clusters[i].medoid = allPoints[i];
        clusters[i].allPoints.clear();
    }

    int worker, val;
    int * th_ids;
    pthread_t threads[N_THREADS];
    th_ids = (int *) malloc(N_THREADS * sizeof(int));
    for(worker = 0; worker < N_THREADS; worker++) {
        th_ids[worker] = worker;
        ClusterPointsWorkerArgs* workerArgs = new ClusterPointsWorkerArgs(worker, N_THREADS, numPoints, numMedoids, medoids, medoidInds, allPoints);
        val = pthread_create(&threads[worker], NULL, clusterPointsWorker,
            (void *) workerArgs);

        /* Check for successful thread creation */
        if(val != 0) {
        printf("Error creating thread #%d, val = %d\n", worker, val);
        exit(-1);
        }
    }
    for(worker = 0; worker < N_THREADS; worker++) {
        pthread_join(threads[worker], NULL);
    }
    free(th_ids);

    for (int i = 0; i < numPoints; i++) {
        clusters[medoidInds[i]].allPoints.push_back(allPoints[i]);
    }
    return clusters;
}

bool clusterPointsIteration(int numMedoids, int numPoints, std::vector<Cluster> &clusters, std::vector<Point> allPoints, int* &medoidInds) {
    bool convergence = true;
    int initialMedoidInds[numPoints] = {0};
    Point medoids[numMedoids];
    memcpy(initialMedoidInds, medoidInds,  numPoints * sizeof(int));
    for (int i = 0; i < numMedoids; i++) {
        clusters[i].allPoints.clear();
        medoids[i] = clusters[i].medoid;
    }

    int worker, val;
    int * th_ids;
    pthread_t threads[N_THREADS];
    th_ids = (int *) malloc(N_THREADS * sizeof(int));
    for(worker = 0; worker < N_THREADS; worker++) {
        th_ids[worker] = worker;
        ClusterPointsWorkerArgs* workerArgs = new ClusterPointsWorkerArgs(worker, N_THREADS, numPoints, numMedoids, medoids, medoidInds, allPoints);
        val = pthread_create(&threads[worker], NULL, clusterPointsWorker,
            (void *) workerArgs);

        /* Check for successful thread creation */
        if(val != 0) {
        printf("Error creating thread #%d, val = %d\n", worker, val);
        exit(-1);
        }
    }
    for(worker = 0; worker < N_THREADS; worker++) {
        pthread_join(threads[worker], NULL);
    }
    free(th_ids);

    for (int i = 0; i < numPoints; i++) {
        if(initialMedoidInds[i] != medoidInds[i]) {
            convergence = false;
        }
        clusters[medoidInds[i]].allPoints.push_back(allPoints[i]);
    }
    return convergence;
}

void assignMedoids(std::vector<Cluster> &clusters, int numClusters) {
    for (int i = 0; i < numClusters; i++) {
        Cluster &c = clusters[i];
        int pointsInCluster = c.allPoints.size();
        double* dists = new double[pointsInCluster];
        int worker, val;
        int * th_ids;
        
        int threadsToUse = N_THREADS < pointsInCluster ? N_THREADS : pointsInCluster;
        pthread_t threads[threadsToUse];
        th_ids = (int *) malloc(N_THREADS * sizeof(int));
        for(worker = 0; worker < threadsToUse; worker++) {
            th_ids[worker] = worker;
            MedoidAssignmentWorkerArgs* workerArgs = new MedoidAssignmentWorkerArgs(worker, threadsToUse, pointsInCluster, dists, c);
            val = pthread_create(&threads[worker], NULL, medoidAssingmentWorker,
                (void *) workerArgs);

            /* Check for successful thread creation */
            if(val != 0) {
            printf("Error creating thread #%d, val = %d\n", worker, val);
            exit(-1);
            }
        }
        for(worker = 0; worker < threadsToUse; worker++) {
            pthread_join(threads[worker], NULL);
        }
        free(th_ids);
        double minDist = dists[0];
        int minDistInd = 0;
        for (int j = 1; j < pointsInCluster; j++) {
            if(dists[j] < minDist) {
                minDist = dists[j];
                minDistInd = j;
            }
        }
        c.medoid = c.allPoints[minDistInd];
    }
}

// ------------------------------
// Main
//-------------------------------

int main(int argc, char** argv){

	// parse arguments
    char* fileName = argv[1];
    int numGroups = atoi(argv[2]);
    int numThreads = atoi(argv[3]);
    N_THREADS = numThreads;

    int numPoints;
    int dimension;

    double* allCoordComponents = ParseFile(fileName, numPoints, dimension);

    std::vector<Point> allPoints;

    for(int i = 0; i < numPoints; i++) {
        double* pointCoords = new double[dimension];
        memcpy(pointCoords, &allCoordComponents[i * dimension],  dimension * sizeof(double));
        allPoints.push_back(*(new Point(dimension, pointCoords)));
    }
    delete[] allCoordComponents;

    // end of file I/O, start of clustering
    double start = monotonic_seconds();
    Point medoids[numGroups];
    int* medoidInds = new int[numPoints]();
    for (int i = 0; i < numGroups; i++) {
        medoids[i] = allPoints[i];
    }
    //max iterations = 20, 1 intial, 19 iterations
    std::vector<Cluster> clusters = clusterPointsInitial(numGroups, numPoints, medoids, allPoints, medoidInds);
    bool converged = false;
    for (int i = 1; i < 20; i++) {
        converged = false;
        assignMedoids(clusters, numGroups);
        converged = clusterPointsIteration(numGroups, numPoints, clusters, allPoints, medoidInds);
        if(converged) {
            i += 20;
        }
    }
    double stop = monotonic_seconds();
    // end of clustering, start of file output
    std::ofstream clustersFile;
    clustersFile.open("clusters.txt");
    for (int i = 0; i < numPoints; i++) {
        clustersFile << medoidInds[i] << "\n";
    }
    clustersFile.close();

    std::ofstream medoidFile;
    medoidFile.open("medoids.txt");
    medoidFile << std::fixed << std::setprecision(3);
    for (int i = 0; i < numGroups; i++) {
        for (int j = 0; j < dimension - 1; j++) {
            double val = clusters[i].medoid.coordinates[j];
            if(val > 1.0 || val < -1.0) {
                medoidFile  << std::fixed << std::setprecision(3) << val << " ";
            }
            else {
                medoidFile.unsetf(std::ios_base::fixed);
                medoidFile << val << " ";
            }
        }
        double fVal = clusters[i].medoid.coordinates[dimension - 1];
        if(fVal > 1.0 || fVal < -1.0) {
            medoidFile  << std::fixed << std::setprecision(3) << fVal << "\n";
        }
        else {
            medoidFile.unsetf(std::ios_base::fixed);
            medoidFile << fVal << "\n";
        }
    }
    medoidFile.close();

    double clusteringTime = stop - start;
    print_time(clusteringTime);
}