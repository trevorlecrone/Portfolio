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
#include "km_kernels.cu"


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

int main(int argc, char** argv){

	// parse arguments
    char* fileName = argv[1];
    int numGroups = atoi(argv[2]);
    int numBlocks = atoi(argv[3]);
    int threadsPerBlock = atoi(argv[4]);

    int numPoints;
    int dimension;
    bool CALC_BLOCKS = (numBlocks == -1);
    double* allCoordComponents = ParseFile(fileName, numPoints, dimension);

    std::vector<Point> allPoints;

    for(int i = 0; i < numPoints; i++) {
        double* pointCoords = new double[dimension];
        memcpy(pointCoords, &allCoordComponents[i * dimension],  dimension * sizeof(double));
        allPoints.push_back(*(new Point(dimension, pointCoords)));
    }

    Point* pointsAsArray = allPoints.data();
    allPoints.clear();

    Point* cudaPoints;
    int pointsMemSize = (numPoints * sizeof(Point));
    cudaMalloc((void**) &cudaPoints, pointsMemSize);
    cudaMemcpy(cudaPoints, pointsAsArray, pointsMemSize, cudaMemcpyHostToDevice);
    printf("cudaMemCopy done\n");

    cudaDeviceSynchronize();
    double start = monotonic_seconds();
    int* medoidInds = new int[numPoints]();
    Point* medoids = new Point[numGroups];
    for (int i = 0; i < numGroups; i++) {
        medoids[i] = allPoints[i];
    }
    delete[] allCoordComponents;

    int* cudaMedoidInds;
    Point* cudaMedoids;
    cudaMalloc(&cudaMedoidInds, numPoints * sizeof(int));
    cudaMemcpy(cudaMedoidInds, medoidInds, numPoints * sizeof(int), cudaMemcpyHostToDevice);
    cudaMalloc(&cudaMedoids, numGroups * sizeof(Point));
    cudaMemcpy(cudaMedoids, medoids, numGroups * sizeof(Point), cudaMemcpyHostToDevice);
    
    //printf("Cuda mem copied\n");
    cudaDeviceSynchronize();
    double start_iter = monotonic_seconds();
    bool converged = false;
    if(CALC_BLOCKS) {
        numBlocks = (numPoints + threadsPerBlock - 1) / threadsPerBlock;
    }
    callClusterPointsKernel(numGroups, numPoints, cudaMedoids, cudaPoints, dimension, cudaMedoidInds, converged, numBlocks, threadsPerBlock);
    cudaDeviceSynchronize();
    double stop_iter = monotonic_seconds();
    printf("initial clustering time: %0.04fs\n", stop_iter - start_iter);
    //max iterations = 20
    for (int i = 0; i < 20; i++) {
        converged = false;
        cudaDeviceSynchronize();
        double start_medoid = monotonic_seconds();
        for (int j = 0; j < numGroups; j++) {
            thrust::device_vector<int> cudaPointFlags(numPoints);
            if(CALC_BLOCKS) {
                numBlocks = (numPoints + threadsPerBlock - 1) / threadsPerBlock;
            }
            callFlagPointsInClusterKernel(numPoints, j, cudaMedoidInds, cudaPointFlags, numBlocks, threadsPerBlock);

            int pointsInCluster = thrust::reduce(cudaPointFlags.begin(), cudaPointFlags.end(), 0, thrust::plus<int>());
            thrust::device_vector<int> indices(pointsInCluster);
            // compute indices of nonzero elements
            using IndexIterator = thrust::device_vector<int>::iterator;
            IndexIterator indices_end = thrust::copy_if(thrust::make_counting_iterator(0), thrust::make_counting_iterator(numPoints), cudaPointFlags.begin(), indices.begin(), thrust::identity<int>());
            //printf("gotIndices");                
            double* cudaDists;
            cudaMalloc(&cudaDists, pointsInCluster * sizeof(double));
            if(CALC_BLOCKS) {
                numBlocks = std::max(1, (pointsInCluster + threadsPerBlock - 1) / threadsPerBlock);
            }
            callGenerateDistsAndAssignMedoidsKernel(dimension, j, cudaPoints, cudaMedoids, cudaDists, pointsInCluster, indices, numBlocks, threadsPerBlock);
            cudaFree(cudaDists);
        }
        cudaDeviceSynchronize();
        double stop_medoid = monotonic_seconds();
        cudaDeviceSynchronize();
        double start_iter = monotonic_seconds();
        if(CALC_BLOCKS) {
            numBlocks = (numPoints + threadsPerBlock - 1) / threadsPerBlock;
        }
        callClusterPointsKernel(numGroups, numPoints, cudaMedoids, cudaPoints, dimension, cudaMedoidInds, converged, numBlocks, threadsPerBlock);
        cudaDeviceSynchronize();
        double stop_iter = monotonic_seconds();
        printf("medoid calculation time: %0.04fs\n", stop_medoid - start_medoid);
        printf("iteration clustering time: %0.04fs\n", stop_iter - start_iter);
        if(converged) {
            i += 20;
        }
    }
    cudaDeviceSynchronize();
    double stop = monotonic_seconds();
    double clusteringTime = stop - start;
    print_time(clusteringTime);

    cudaMemcpy(medoids, cudaMedoids, numGroups * sizeof(Point), cudaMemcpyDeviceToHost);
    cudaMemcpy(medoidInds, cudaMedoidInds, numPoints * sizeof(int), cudaMemcpyDeviceToHost);

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