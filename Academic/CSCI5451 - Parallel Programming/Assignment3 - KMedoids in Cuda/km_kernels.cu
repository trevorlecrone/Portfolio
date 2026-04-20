
#pragma once
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "Point.h"
#include <thrust/iterator/counting_iterator.h>
#include <thrust/copy.h>
#include <thrust/functional.h>
#include <thrust/device_vector.h>
#include <thrust/host_vector.h>

#include <stdio.h>


__device__ double distanceTo(const double* p1, const double* p2, int dimensions) {
    double sum = 0.0;
    for (int i = 0; i < dimensions; i++) {
        double distComp = p1[i] - p2[i];
        sum += (distComp * distComp);
    }
    return sum;
}

__global__ void setCoords(int clusterId, cub::KeyValuePair<int, double>* kvp, Point* medoids, Point* allPoints, thrust::device_vector<int> pointIndices) {
    int minIndex = kvp->key;
    int medoidInd = pointIndices[minIndex];
    medoids[clusterId].coordinates = allPoints[medoidInd].coordinates;
}

__global__ void clusterPoints(int numMedoids, int numPoints, Point* medoids, Point* allPoints, int dimensions, int* &medoidInds, bool &convergence) {
    extern __shared__ Point sharedPoints[];
    for (int p = threadIdx.x; p < numMedoids; p += blockDim.x) {
        sharedPoints[p] = medoids[p];
    }
    __syncthreads();

    int index = blockIdx.x * blockDim.x + threadIdx.x;
    int stride = blockDim.x * gridDim.x;
    for (int i = index; i < numPoints; i += stride) {
        double minDistance = 1000000000.0;
        int minDistInd = 0;
        for (int j = 0; j < numMedoids; j++) {
            double distance = distanceTo(allPoints[i].coordinates, sharedPoints[j].coordinates, dimensions);
            if (distance <= minDistance) {
                minDistance = distance;
                minDistInd = j;
            }
        }
        if (medoidInds[i] != minDistInd) {
            medoidInds[i] = minDistInd;
            convergence = false;
        }
    }
}

__global__ void flagPointsInCluster(int numPoints, int clusterId, int* medoidInds, thrust::device_vector<int> &pointFlags) {
    int index = blockIdx.x * blockDim.x + threadIdx.x;
    int stride = blockDim.x * gridDim.x;

    for (int i = index; i < numPoints; i += stride) {
        if (medoidInds[i] == clusterId) {
            pointFlags[i] = 1;
        }
    }
}

__global__ void generateDists(int dimensions, int clusterId, Point* allPoints, double* &dists, int pointsInCluster, thrust::device_vector<int> pointIndices) {
    extern __shared__ Point sharedPoints[];
    for (int p = threadIdx.x; p < pointsInCluster; p += blockDim.x) {
        int pointIndex = pointIndices[p];
        sharedPoints[p] = allPoints[pointIndex];
    }
    __syncthreads();

    int index = blockIdx.x * blockDim.x + threadIdx.x;
    int stride = blockDim.x * gridDim.x;
    for (int p = index; p < pointsInCluster; p += stride) {
        double totalDist = 0.0;
        for (int p2 = 0; p2 < pointsInCluster; p2++) {
            totalDist += distanceTo(sharedPoints[p].coordinates, sharedPoints[p2].coordinates, dimensions);
        }
        dists[p] = totalDist;
    }
}

void callClusterPointsKernel(int numMedoids, int numPoints, Point* medoids, Point* allPoints, int dimensions, int* &medoidInds, bool &convergence, int numBlocks, int threadsPerBlock) {
    size_t sharedMemorySize = numMedoids * sizeof(Point);
    clusterPoints<<<numBlocks, threadsPerBlock, sharedMemorySize>>>(numMedoids, numPoints, medoids, allPoints, dimensions, medoidInds, convergence);
}

void callFlagPointsInClusterKernel( int numPoints, int clusterId, int* medoidInds, thrust::device_vector<int> &pointFlags, int numBlocks, int threadsPerBlock) {
    flagPointsInCluster<<<numBlocks, threadsPerBlock>>>(numPoints, clusterId, medoidInds, pointFlags);
}

void callGenerateDistsAndAssignMedoidsKernel(int dimensions, int clusterId, Point* allPoints, Point* medoids, double* &dists, int pointsInCluster, thrust::device_vector<int> pointIndices, int numBlocks, int threadsPerBlock) {
    size_t sharedMemorySize = pointsInCluster * sizeof(Point);
    generateDists<<<numBlocks, threadsPerBlock, sharedMemorySize>>>(dimensions, clusterId, allPoints, dists, pointsInCluster, pointIndices);
    cudaError_t err = cudaGetLastError();
    if (err != cudaSuccess) {
        printf("CUDA error: %s\n", cudaGetErrorString(err));
        return;
    }
    cub::KeyValuePair<int, double> *h_argmin = new cub::KeyValuePair<int, double>();
    cub::KeyValuePair<int, double> *d_argmin;
    cudaMalloc(&d_argmin, sizeof(cub::KeyValuePair<int, double>));
    // Determine temporary device storage requirements
    void *d_temp_storage = NULL;
    size_t temp_storage_bytes = 0;
    cub::DeviceReduce::ArgMin(d_temp_storage, temp_storage_bytes, dists, d_argmin, pointsInCluster);

    // Allocate temporary storage
    cudaMalloc(&d_temp_storage, pointsInCluster * sizeof(double));

    // Run argmin-reduction
    cub::DeviceReduce::ArgMin(d_temp_storage, temp_storage_bytes, dists, d_argmin, pointsInCluster);
    setCoords<<<1,1>>>(clusterId, d_argmin, medoids, allPoints, pointIndices);
}


