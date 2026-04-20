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
#include "mpi.h"

int BASE_LENGTH = 0;

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
* @param seconds Seconds spent on quick sorting array clustering, excluding IO.
*/
static void print_time(double const seconds)
{
  printf("quick sort time: %0.04fs\n", seconds);
}

/**
* @brief Write an array of integers to a file.
*
* @param filename The name of the file to write to.
* @param numbers The array of numbers.
* @param nnumbers How many numbers to write.
*/
static void print_numbers(
    char const * const filename,
    int const * const numbers,
    int const nnumbers)
{
  FILE * fout;

  /* open file */
  if((fout = fopen(filename, "w")) == NULL) {
    fprintf(stderr, "error opening '%s'\n", filename);
    abort();
  }

  /* write the header */
  fprintf(fout, "%d\n", nnumbers);

  /* write numbers to fout */
  for(int i = 0; i < nnumbers; ++i) {
    fprintf(fout, "%d\n", numbers[i]);
  }

  fclose(fout);
}

//Camparison operator needed for C++ std::qsort
int compare (const void * a, const void * b)
{
  return ( *(int*)a - *(int*)b );
}

int getMedian(int* array, int len){
    std::qsort(array, len, sizeof(int), compare);

    return array[len/2];
}

int* sendAndReceiveToLTERanks(int lteProcesses, int rank, int* prefixSumLessOrEqualSize, int processes, std::vector<int> lessThanOrEqual, int& size, MPI_Comm comm) {
    MPI_Comm localComm;
    MPI_Comm_dup(comm, &localComm);
    int lteElements = prefixSumLessOrEqualSize[processes];
    int lteElementsPerProcess = lteElements / lteProcesses;
    int lteRemainder = lteElements % lteProcesses;

    //first do sends
    int firstSendIndex = prefixSumLessOrEqualSize[rank];
    int lastSendIndex = prefixSumLessOrEqualSize[rank + 1] - 1;

    //send "down" ranks, all processors can do this
    int firstSendLimit = lteElementsPerProcess * rank;
    if (rank >= lteProcesses) {
        firstSendLimit += lteRemainder;
    }
    while (firstSendIndex < firstSendLimit && firstSendIndex <= lastSendIndex) {
        int rankToSendTo = std::min(firstSendIndex / lteElementsPerProcess, lteProcesses - 1);
        int totalUnsentElements = (lastSendIndex - firstSendIndex + 1);
        int elementsToSend = std::min(((rankToSendTo + 1) * lteElementsPerProcess) - firstSendIndex, totalUnsentElements);
        if (rankToSendTo == lteProcesses - 1) {
            elementsToSend = std::min(((rankToSendTo + 1) * lteElementsPerProcess + lteRemainder) - firstSendIndex, totalUnsentElements);
        }
        if (elementsToSend == 0) {
            //printf("rank %d breaks, firstSendIndex = %d lastSendIndex = %d\n", rank, firstSendIndex, lastSendIndex);
            firstSendIndex = lastSendIndex + 1;
            break;
        }

        int localStart = firstSendIndex - prefixSumLessOrEqualSize[rank];
        std::vector<int> sendSubVector = std::vector<int>(lessThanOrEqual.begin() + localStart, lessThanOrEqual.begin() + localStart + elementsToSend);
        int* sendBuffer = (int*) &sendSubVector[0];
        MPI_Send(sendBuffer, elementsToSend, MPI_INT, rankToSendTo, 0, localComm);
        //printf("rank %d sends %d elements from lessThan starting at index %d to rank %d\n", rank, elementsToSend, localStart, rankToSendTo); 
        firstSendIndex += elementsToSend;
    }
    // only receiving processors
    if (rank < lteProcesses) {
        int numElements = lteElementsPerProcess;
        if (rank == lteProcesses - 1) {
            numElements += lteRemainder;
        }
        size = numElements;
        //printf("rank %d numElements %d\n", rank, numElements);
        int* result = new int[numElements];
        int resultIndex = 0;
        // copy relevant data to local mem
        if(firstSendIndex <= lastSendIndex) {
            int localStart = firstSendIndex - prefixSumLessOrEqualSize[rank];
            int elementsToCopy = std::min(lastSendIndex - firstSendIndex + 1, numElements);
            std::vector<int> copySubVector = std::vector<int>(lessThanOrEqual.begin() + localStart, lessThanOrEqual.begin() + localStart + elementsToCopy);
            int* copyBuffer = (int*) &copySubVector[0];
            //printf("rank %d copies %d elements from lessThan starting at index %d to local mem\n", rank, elementsToCopy, localStart);
            memcpy(result, copyBuffer, elementsToCopy * sizeof(int));
            firstSendIndex += elementsToCopy;
            resultIndex += elementsToCopy;
        }
        //send "up" ranks if process as too many elements
        //printf("rank %d firstSendIndex %d lastSendIndex %d\n", rank, firstSendIndex, lastSendIndex);
        if(firstSendIndex <= lastSendIndex) {
            while (lastSendIndex >= lteElementsPerProcess * rank + numElements) {
                int rankToSendTo  = lastSendIndex / lteElementsPerProcess;
                int totalUnsentElements = (lastSendIndex - firstSendIndex + 1);
                int elementsToSend = std::min(lastSendIndex - ((rankToSendTo * lteElementsPerProcess) - 1), totalUnsentElements);
                if (rankToSendTo == lteProcesses - 1) {
                    elementsToSend = std::min(lastSendIndex - ((rankToSendTo * lteElementsPerProcess) - 1) + lteRemainder, totalUnsentElements);
                }

                int localEnd = lastSendIndex - prefixSumLessOrEqualSize[rank] + 1;
                std::vector<int> sendSubVector = std::vector<int>(lessThanOrEqual.begin() + localEnd - elementsToSend, lessThanOrEqual.begin() + localEnd);
                int* sendBuffer = &sendSubVector[0];
                MPI_Send(sendBuffer, elementsToSend, MPI_INT, rankToSendTo, 0, localComm);
                //printf("rank %d sends %d elements from lessThan ending at index %d to rank %d\n", rank, elementsToSend, localEnd, rankToSendTo);
                lastSendIndex -= elementsToSend;
            }
        }
        // receive from other processes.
        int firstProcessElementIndex = rank * lteElementsPerProcess;
        int lastProcessElementIndex = firstProcessElementIndex + numElements - 1;
        int totalUnreceivedElemenets = (numElements - resultIndex);
        for (int i = 0; i < processes; i++) {
            int rankIFirstIndex = prefixSumLessOrEqualSize[i];
            int rankILastIndex = prefixSumLessOrEqualSize[i + 1] - 1;
            int elementsInRank = rankILastIndex - rankIFirstIndex + 1;
            if(elementsInRank > 0){
                if(i < rank && rankILastIndex >= firstProcessElementIndex && totalUnreceivedElemenets > 0) {
                    int validElementsInRankI = std::min(rankILastIndex - firstProcessElementIndex + 1, elementsInRank);
                    int elementsToReceive = std::min(validElementsInRankI, totalUnreceivedElemenets);
                    //printf("rank %d receives %d elements from rank %d\n", rank, elementsToReceive, i);
                    MPI_Recv(&result[resultIndex], elementsToReceive, MPI_INT, i, 0, localComm, MPI_STATUS_IGNORE);
                    resultIndex += elementsToReceive;
                    firstProcessElementIndex += elementsToReceive;
                    totalUnreceivedElemenets -= elementsToReceive;
                }
                if(i > rank && rankIFirstIndex <= lastProcessElementIndex && totalUnreceivedElemenets > 0) {
                    int elementsBoundForLowerRanks = std::max(firstProcessElementIndex - rankIFirstIndex, 0);
                    int validElementsInRankI = std::min(lastProcessElementIndex - rankIFirstIndex + 1, elementsInRank - elementsBoundForLowerRanks);
                    if(validElementsInRankI > 0) {
                        //printf("rank %d, firstProcessElementIndex %d, lastProcessElementIndex %d, rankIFirstIndex %d, elementsInRank %d\n", rank, firstProcessElementIndex, lastProcessElementIndex, rankIFirstIndex, elementsInRank);
                        int elementsToReceive = std::min(validElementsInRankI, totalUnreceivedElemenets);
                        //printf("rank %d receives %d elements from rank, %d\n", rank, elementsToReceive, i);
                        MPI_Recv(&result[resultIndex], elementsToReceive, MPI_INT, i, 0, localComm, MPI_STATUS_IGNORE);
                        resultIndex += elementsToReceive;
                        firstProcessElementIndex += elementsToReceive;
                        totalUnreceivedElemenets -= elementsToReceive;
                    }
                }
            }
        }
        return result;
    }
    return (int*)-1;
}

int* sendAndReceiveToGreaterThanRanks(int gtProcesses, int rank, int* prefixSumGreaterThanSize, int processes, std::vector<int> greaterThan, int& size, MPI_Comm comm) {
    MPI_Comm localComm;
    MPI_Comm_dup(comm, &localComm);
    int gtElements = prefixSumGreaterThanSize[processes];
    int gtElementsPerProcess = gtElements / gtProcesses;
    int gtRemainder = gtElements % gtProcesses;
    int ltProcesses = processes - gtProcesses;

    //first do sends
    int adjuster = gtElementsPerProcess * ltProcesses;
    int firstSendIndex = prefixSumGreaterThanSize[rank];
    int lastSendIndex = prefixSumGreaterThanSize[rank + 1] - 1;
    int adFirstSendIndex = firstSendIndex + adjuster;
    int adLastSendIndex = lastSendIndex + adjuster;
    //send "up" ranks, all processors can do this
    int topEndRank = processes - (rank + 1);
    int lastSendLimit = rank < ltProcesses ? adjuster - 1 : gtElementsPerProcess * (rank + 1) - 1;
    if (rank == processes - 1) {
        lastSendLimit += gtRemainder;
    }
    
    //printf("rank %d lastSendLimit %d\n", rank, lastSendLimit);
    //printf("rank %d adLastSendIndex %d, adFirstSendIndex %d\n", rank, adLastSendIndex, adFirstSendIndex);

    while (adLastSendIndex > lastSendLimit && adLastSendIndex >= adFirstSendIndex) {
        int rankToSendTo = std::min(adLastSendIndex / gtElementsPerProcess, processes - 1);
        int totalUnsentElements = (adLastSendIndex - adFirstSendIndex + 1);
        int elementsToSend = std::min(adLastSendIndex - ((rankToSendTo * gtElementsPerProcess) - 1), totalUnsentElements);
        //printf("rank %d totalUnsentElements %d, elementsToSend %d\n", rank, totalUnsentElements, elementsToSend);
        if (elementsToSend <= 0) {
            //printf("rank %d breaks, adFirstSendIndex = %d adLastSendIndex = %d\n", rank, adFirstSendIndex, adLastSendIndex);
            adFirstSendIndex = adLastSendIndex +1;
            break;
        }
        int localEnd = lastSendIndex - prefixSumGreaterThanSize[rank] + 1;
        std::vector<int> sendSubVector = std::vector<int>(greaterThan.begin() + localEnd - elementsToSend, greaterThan.begin() + localEnd);
        int* sendBuffer = (int*) &sendSubVector[0];
        MPI_Send(sendBuffer, elementsToSend, MPI_INT, rankToSendTo, 0, localComm);
        //printf("rank %d sends %d elements from greaterThan ending at index %d to rank %d\n", rank, elementsToSend, localEnd - 1, rankToSendTo); 
        lastSendIndex -= elementsToSend;
        adLastSendIndex -= elementsToSend;
    }
    // only receiving processors
    if (rank >= ltProcesses) {
        int numElements = gtElementsPerProcess;
        if (rank == processes - 1) {
            numElements += gtRemainder;
        }
        //printf("rank %d numElements %d\n", rank, numElements);
        size = numElements;
        int* result = new int[numElements];
        int resultIndex = 0;
        // copy relevant data to local mem
        //printf("rank %d, lastSendIndex %d, prefixSumGreaterThanSize[rank] %d\n", rank, lastSendIndex, prefixSumGreaterThanSize[rank]);
        if(lastSendIndex >= firstSendIndex) {
            int localEnd = lastSendIndex - prefixSumGreaterThanSize[rank] + 1;
            //printf("rank %d localEnd %d\n", rank, localEnd);
            int elementsToCopy = std::min(lastSendIndex - firstSendIndex + 1, numElements);
            std::vector<int> copySubVector = std::vector<int>(greaterThan.begin() + localEnd - elementsToCopy, greaterThan.begin() + localEnd);
            int* copyBuffer = (int*) &copySubVector[0];
            //printf("rank %d copies %d elements from greaterThan ending at index %d to local mem\n", rank, elementsToCopy, localEnd);
            memcpy(result, copyBuffer, elementsToCopy * sizeof(int));
            lastSendIndex -= elementsToCopy;
            adLastSendIndex -= elementsToCopy;
            resultIndex += elementsToCopy;
        }
        //printf("rank %d adFirstSendIndex %d adLastSendIndex %d\n", rank, adFirstSendIndex, adLastSendIndex);
        //send "down" ranks if process as too many elements
        if (adFirstSendIndex <= adLastSendIndex) {
            while (adFirstSendIndex < (gtElementsPerProcess * rank)) {
                int rankToSendTo  = adFirstSendIndex / gtElementsPerProcess;
                int totalUnsentElements = (adLastSendIndex - adFirstSendIndex + 1);
                int elementsToSend = std::min(((rankToSendTo + 1) * gtElementsPerProcess) - adFirstSendIndex, totalUnsentElements);
                if (rankToSendTo == processes - 1) {
                    elementsToSend = std::min(((rankToSendTo + 1) * gtElementsPerProcess + gtRemainder) - firstSendIndex, totalUnsentElements);
                }

                int localStart = adFirstSendIndex - (prefixSumGreaterThanSize[rank] + adjuster);
                std::vector<int> sendSubVector = std::vector<int>(greaterThan.begin() + localStart, greaterThan.begin() + localStart + elementsToSend);
                int* sendBuffer = (int*) &sendSubVector[0];
                MPI_Send(sendBuffer, elementsToSend, MPI_INT, rankToSendTo, 0, localComm);
                //printf("rank %d sends %d elements from greaterThan starting at index %d to rank %d\n", rank, elementsToSend, localStart, rankToSendTo);
                adFirstSendIndex += elementsToSend;
            }
        }
        // receive from other processes.
        int relativeRank = rank - ltProcesses;
        int firstProcessElementIndex = relativeRank * gtElementsPerProcess;
        int lastProcessElementIndex = firstProcessElementIndex + numElements - 1;
        int totalUnreceivedElemenets = (numElements - resultIndex);
        for (int i = 0; i < processes; i++) {
            int rankIFirstIndex = prefixSumGreaterThanSize[i];
            int rankILastIndex = prefixSumGreaterThanSize[i + 1] - 1;
            int elementsInRank = rankILastIndex - rankIFirstIndex + 1;
            if(elementsInRank > 0){
                if(i < rank && rankILastIndex >= firstProcessElementIndex && totalUnreceivedElemenets > 0) {
                    int elementsBoundForHigherRanks = std::max(rankILastIndex - lastProcessElementIndex, 0);
                    int validElementsInRankI = std::min((rankILastIndex - firstProcessElementIndex + 1), elementsInRank - elementsBoundForHigherRanks);
                    if(validElementsInRankI > 0) {
                        int elementsToReceive = std::min(validElementsInRankI, totalUnreceivedElemenets);
                        //printf("rank %d resultIndex %d \n", rank, resultIndex);
                        //printf("rank %d receives %d gt elements from rank %d\n", rank, elementsToReceive, i);
                        MPI_Recv(&result[resultIndex], elementsToReceive, MPI_INT, i, 0, localComm, MPI_STATUS_IGNORE);
                        resultIndex += elementsToReceive;
                        firstProcessElementIndex += elementsToReceive;
                        totalUnreceivedElemenets -= elementsToReceive;
                    }
                }
                if(i > rank && rankIFirstIndex <= lastProcessElementIndex && totalUnreceivedElemenets > 0) {
                    int validElementsInRankI = std::min(lastProcessElementIndex - rankIFirstIndex + 1, elementsInRank);
                    int elementsToReceive = std::min(validElementsInRankI, totalUnreceivedElemenets);
                    //printf("rank %d receives %d gt elements from rank %d\n", rank, elementsToReceive, i);
                    MPI_Recv(&result[resultIndex], elementsToReceive, MPI_INT, i, 0, localComm, MPI_STATUS_IGNORE);
                    resultIndex += elementsToReceive;
                    firstProcessElementIndex += elementsToReceive;
                    totalUnreceivedElemenets -= elementsToReceive;
                }
            }
        }
        return result;
    }
    return (int*) -1;
}

int* balanceResults(int* qsResult, int* prefixSumResultSize, int processes, int rank, MPI_Comm comm) {
    MPI_Comm localComm;
    MPI_Comm_dup(comm, &localComm);
    int* result = new int[BASE_LENGTH];
    int resultIndex = 0;
    // first send "up", so any processes that need values lower than they currently have can get them at the front of their array
    int myFirstIndex = prefixSumResultSize[rank];
    int myLastIndex = prefixSumResultSize[rank + 1] - 1;
    int myTargetFirstIndex = rank * BASE_LENGTH;
    int myTargetLastIndex = ((rank + 1) * BASE_LENGTH) - 1;
    for (int i = processes - 1; i >= 0; i--) {
        int rankITargetFirstIndex = i * BASE_LENGTH;
        int rankIFirstIndex = prefixSumResultSize[i];
        int rankILastIndex = prefixSumResultSize[i + 1] - 1;
        if(i > rank && myLastIndex >= rankITargetFirstIndex) {
            int elementsToSend = myLastIndex - rankITargetFirstIndex + 1;
            int bufferStartIndex = (myLastIndex - myFirstIndex) - (elementsToSend - 1);
            //printf("rank %d send %d elements to rank %d, bufferStartIndex %d\n", rank, elementsToSend, i, bufferStartIndex);
            MPI_Send(&qsResult[bufferStartIndex], elementsToSend, MPI_INT, i, 0, localComm);
            myLastIndex -= elementsToSend;
        }
        if(i < rank && rankILastIndex >= myTargetFirstIndex) {
            int elementsToRecv = rankILastIndex - myTargetFirstIndex + 1;
            //printf("rank %d receives %d elements from rank %d\n", rank, elementsToRecv, i);
            MPI_Recv(&result[resultIndex], elementsToRecv, MPI_INT, i, 0, localComm, MPI_STATUS_IGNORE);
            resultIndex += elementsToRecv;
        }
    }
    // copy relevant data into local mem
    int offset = 0;
    int elementsToCopy = 0;
    for (int i = myFirstIndex; i <= myLastIndex; i++) {
        if(i >= myTargetFirstIndex) {
            if(i <= myTargetLastIndex) {
                elementsToCopy++;
            }
        }
        else {
            offset++;
        }
    }
    //printf("rank %d copies %d elements from qsResult starting at index %d to local mem\n", rank, elementsToCopy, offset);
    int* copyBuffer = (int*) &qsResult[offset];
    memcpy(result + resultIndex, copyBuffer, elementsToCopy * sizeof(int));
    resultIndex += elementsToCopy;

    // now send "down", processes have larger elements at the end of their arrays
    for (int i = 0; i < processes; i++) {
        int rankITargetLastIndex = ((i + 1) * BASE_LENGTH) - 1;
        int rankIFirstIndex = prefixSumResultSize[i];
        int rankILastIndex = prefixSumResultSize[i + 1] - 1;
        if(i < rank && myFirstIndex <= rankITargetLastIndex) {
            int elementsToSend = rankITargetLastIndex - myFirstIndex + 1;
            //printf("rank %d send %d elements to rank %d\n", rank, elementsToSend, i);
            MPI_Send(&qsResult[0], elementsToSend, MPI_INT, i, 0, localComm);
            myFirstIndex += elementsToSend;
        }
        if(i > rank && rankIFirstIndex <= myTargetLastIndex) {
            int elementsToRecv = myTargetLastIndex - rankIFirstIndex + 1;
            //printf("rank %d receives %d elements from rank %d\n", rank, elementsToRecv, i);
            MPI_Recv(&result[resultIndex], elementsToRecv, MPI_INT, i, 0, localComm, MPI_STATUS_IGNORE);
            resultIndex += elementsToRecv;
        }
    }
    return result;
}

int* quickSortMPI(int* array, int &len, MPI_Comm comm) {
    int processes;
    int worldProcesses;
    int rank; 
    MPI_Comm localComm;
    MPI_Comm_dup(comm, &localComm);
    MPI_Comm_size(localComm, &processes);
    MPI_Comm_size(MPI_COMM_WORLD, &worldProcesses);
    MPI_Comm_rank(localComm, &rank);
    if(processes == 1) {
        //printf("rank %d: Done, len %d\n", rank, len);
        std::qsort(array, len, sizeof(int), compare);
        return array;
    }
    //get random val from all processes, re seed rand with rank as seed was lost
    /* srand(rank);
    int randVal = array[rand() % (len + 1)]; */
    // use mean from each processor rather than median, faster to calculate while still giving better load balancing than random values
    long long sum = 0;
    for (int i = 0; i < len; i++) {
        long long val = (long long) array[i];
        sum += val;
        //if(i % 100000 == 0){printf("rank %d sum %lld\n", rank, sum);}
    }
    //printf("rank %d sum %ld", rank, sum);
    long mean = sum / (long)len;
    int* pivotInputs = new int[processes];
    int intMean = (int) mean;
    MPI_Allgather(&intMean, 1, MPI_INT, pivotInputs, 1, MPI_INT, localComm);
    /* printf("rank %d pivotInputs: (", rank);
    for (int i = 0; i < processes; i++)
    {
        printf("%d", pivotInputs[i]);
        printf(i < processes - 1 ? "," : ")\n");
    } */
    int pivot;
    if (rank == 0) {
        pivot = getMedian(pivotInputs, processes);
    }
    MPI_Bcast(&pivot, 1, MPI_INT, 0, localComm);
    
    //printf("rank %d: pivot %d\n", rank, pivot);
    std::vector<int> lessThanOrEqual;
    std::vector<int> greaterThan;
    for (int i = 0; i < len; i++)
    {
        int val = array[i];
        if(val <= pivot) {
            lessThanOrEqual.push_back(val);
        }
        else {
            greaterThan.push_back(val);
        }
    }
    free(array);
    /* //printf("rank %d lessThanOrEqual: (", rank);
    for (int i = 0; i < lessThanOrEqual.size(); i++)
    {
        //printf("%d", lessThanOrEqual[i]);
        //printf(i < lessThanOrEqual.size() - 1 ? "," : "");
    }
    //printf(")\n");
    //printf("rank %d greaterThan: (", rank);
    for (int i = 0; i < greaterThan.size(); i++)
    {
        //printf("%d", greaterThan[i]);
        //printf(i < greaterThan.size() - 1 ? "," : "");
    }
    //printf(")\n"); */
    int procLessOrEqualSize = lessThanOrEqual.size();
    int procGreaterSize = greaterThan.size();
    //printf("rank %d procLessOrEqualSize: %d\n", rank, procLessOrEqualSize);
    //printf("rank %d procGreaterSize: %d\n", rank, procGreaterSize);

    int runningLessOrEqualSize = 0;
    int runningGreaterSize = 0;

    MPI_Scan(&procLessOrEqualSize, &runningLessOrEqualSize, 1, MPI_INT, MPI_SUM, localComm);
    MPI_Scan(&procGreaterSize, &runningGreaterSize, 1, MPI_INT, MPI_SUM, localComm);

    /* printf("rank %d runningLessOrEqualSize: %d\n", rank, runningLessOrEqualSize);
    printf("rank %d runningGreaterSize: %d\n", rank, runningGreaterSize); */

    int lteElements, gtElements, totalElements;
    if (rank == processes - 1) {
        lteElements = runningLessOrEqualSize;
        gtElements = runningGreaterSize;
        totalElements = lteElements + gtElements;
        //printf("runningLessOrEqualSize %d, runningGreaterSize %d\n", lteElements, runningGreaterSize);
    }
    MPI_Bcast(&lteElements, 1, MPI_INT, processes - 1, localComm);
    MPI_Bcast(&gtElements, 1, MPI_INT, processes - 1, localComm);
    MPI_Bcast(&totalElements, 1, MPI_INT, processes - 1, localComm);
    //printf("rank %d totalElements: %d\n", rank, totalElements); 
    
    double lteRatio = ((double)lteElements) / ((double)totalElements);

    int lteProcesses = (int) ((lteRatio * (double) processes) + 0.5);
    lteProcesses = lteProcesses == 0 ? 1 : lteProcesses;
    lteProcesses = lteProcesses == processes ? processes - 1 : lteProcesses;
    
    int gtProcesses = processes - lteProcesses;
    int lteElementsPerProcess = lteElements / lteProcesses;
    int lteRemainder = lteElements % lteProcesses;

    int gtElementsPerProcess = gtElements / gtProcesses;
    int gtRemainder = gtElements % gtProcesses;

    //printf("rank %d lteRemainder: %d\n", rank, lteRemainder);

    int prefixSumLessOrEqualSize[processes + 1];
    int prefixSumGreaterSize[processes + 1];
    prefixSumLessOrEqualSize[0] = 0;
    prefixSumGreaterSize[0] = 0;

    MPI_Allgather(&runningLessOrEqualSize, 1, MPI_INT, &prefixSumLessOrEqualSize[1], 1, MPI_INT, localComm);
    MPI_Allgather(&runningGreaterSize, 1, MPI_INT, &prefixSumGreaterSize[1], 1, MPI_INT, localComm);
    int* newArray;
    int color;
    if(rank < lteProcesses) {
        newArray = sendAndReceiveToLTERanks(lteProcesses, rank, prefixSumLessOrEqualSize, processes, lessThanOrEqual, len, localComm);
        sendAndReceiveToGreaterThanRanks(gtProcesses, rank, prefixSumGreaterSize, processes, greaterThan, len, localComm);
        color = 0;
    }
    else {
        sendAndReceiveToLTERanks(lteProcesses, rank, prefixSumLessOrEqualSize, processes, lessThanOrEqual, len, localComm);
        newArray = sendAndReceiveToGreaterThanRanks(gtProcesses, rank, prefixSumGreaterSize, processes, greaterThan, len, localComm);
        color = 1;
    }
    greaterThan.clear();
    lessThanOrEqual.clear();
    greaterThan.shrink_to_fit();
    lessThanOrEqual.shrink_to_fit();
    /* printf("rank: %d, size %d\n", rank, size);
    for (int i = 0; i < size; i++) {
        printf("rank: %d, result[i] %d, i %d\n", rank, newArray[i], i);
    } */
    MPI_Comm new_comm;
    MPI_Comm_split(localComm, color, rank, &new_comm);
    int* qsresult;
    qsresult = quickSortMPI(newArray, len, new_comm);
    if(processes == worldProcesses) {
        //printf("rank %d: Done, len %d, qsresult[0] %d\n", rank, len, qsresult[0]);
        int runningResultSize = 0;
        MPI_Scan(&len, &runningResultSize, 1, MPI_INT, MPI_SUM, localComm);
        int prefixSumResultSize[processes + 1];
        prefixSumResultSize[0] = 0;
        MPI_Allgather(&runningResultSize, 1, MPI_INT, &prefixSumResultSize[1], 1, MPI_INT, localComm);
        int* balancedResult;
        balancedResult = balanceResults(qsresult, prefixSumResultSize, processes, rank, localComm);
        return balancedResult;
    }
    return qsresult;
}

/* void sendAndReceiveTests(int rank, MPI_Comm comm) {
    MPI_Comm localComm;
    MPI_Comm_dup(comm, &localComm);
    std::vector<int> lessThanOrEqual;
    std::vector<int> greaterThan;
    int processes = 4;
    int gtProcesses = 2;
    int lteProcesses = 2;
    if (rank == 0) {
        for (int i = 0; i < 2; i++) {
            lessThanOrEqual.push_back(0 + i);
        }
    }
    if (rank == 1) {
        for (int i = 0; i < 4; i++) {
            greaterThan.push_back(100+ i);
        }
    }
    if (rank == 2) {
        for (int i = 0; i < 4; i++) {
            lessThanOrEqual.push_back(10 + i);
        }
    }
    if (rank == 3) {
        for (int i = 0; i < 2; i++) {
            greaterThan.push_back(200 + i);
        }
    }
    int runningLessOrEqualSize = 0;
    int runningGreaterSize = 0;

    int procLessOrEqualSize = lessThanOrEqual.size();
    int procGreaterSize = greaterThan.size();

    MPI_Scan(&procLessOrEqualSize, &runningLessOrEqualSize, 1, MPI_INT, MPI_SUM, localComm);
    MPI_Scan(&procGreaterSize, &runningGreaterSize, 1, MPI_INT, MPI_SUM, localComm);

    printf("rank %d runningLessOrEqualSize: %d\n", rank, runningLessOrEqualSize);
    printf("rank %d runningGreaterSize: %d\n", rank, runningGreaterSize);

    int lteElements, gtElements, totalElements;
    if (rank == processes - 1) {
        lteElements = runningLessOrEqualSize;
        gtElements = runningGreaterSize;
        totalElements = lteElements + gtElements;
    }
    MPI_Bcast(&lteElements, 1, MPI_INT, processes - 1, localComm);
    MPI_Bcast(&gtElements, 1, MPI_INT, processes - 1, localComm);
    MPI_Bcast(&totalElements, 1, MPI_INT, processes - 1, localComm);
    int prefixSumLessOrEqualSize[processes + 1];
    int prefixSumGreaterSize[processes + 1];
    prefixSumLessOrEqualSize[0] = 0;
    prefixSumGreaterSize[0] = 0;

    MPI_Allgather(&runningLessOrEqualSize, 1, MPI_INT, &prefixSumLessOrEqualSize[1], 1, MPI_INT, localComm);
    MPI_Allgather(&runningGreaterSize, 1, MPI_INT, &prefixSumGreaterSize[1], 1, MPI_INT, localComm);

    int* newArray;
    int size;
    if(rank < lteProcesses) {
        newArray = sendAndReceiveToLTERanks(lteProcesses, rank, prefixSumLessOrEqualSize, processes, lessThanOrEqual, size, localComm);
        sendAndReceiveToGreaterThanRanks(gtProcesses, rank, prefixSumGreaterSize, processes, greaterThan, size, localComm);
    }
    else {
        sendAndReceiveToLTERanks(lteProcesses, rank, prefixSumLessOrEqualSize, processes, lessThanOrEqual, size, localComm);
        newArray = sendAndReceiveToGreaterThanRanks(gtProcesses, rank, prefixSumGreaterSize, processes, greaterThan, size, localComm);
    }
    //printf("rank: %d, size %d\n", rank, size);
    for (int i = 0; i < size; i++) {
        //printf("rank: %d, result[i] %d, i %d\n", rank, newArray[i], i);
    }
    return;
} */

// ------------------------------
// Main
//-------------------------------

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int processes;
    int rank; 
    MPI_Comm_size(MPI_COMM_WORLD, &processes);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int numVals = atoi(argv[1]);
    int valsPerProcess = numVals/processes;
    BASE_LENGTH = valsPerProcess;

    srand(rank);
    //int* allVals = new int[numVals];
    int* processVals = new int[valsPerProcess];
    for (int i = 0; i < valsPerProcess; i++)
    {
        processVals[i] = rand();
    }

    //MPI_Gather(processVals, BASE_LENGTH, MPI_INT, allVals, BASE_LENGTH, MPI_INT, 0, MPI_COMM_WORLD);

    int* result;
    double start = monotonic_seconds();
    result = quickSortMPI(processVals, valsPerProcess, MPI_COMM_WORLD);
    double stop  = monotonic_seconds();
    int* parSorted = new int[numVals];
    MPI_Gather(result, BASE_LENGTH, MPI_INT, parSorted, BASE_LENGTH, MPI_INT, 0, MPI_COMM_WORLD);
    if(rank == 0) {
        print_time(stop - start);
        //std::qsort(allVals, numVals, sizeof(int), compare);
        /* for (int i = 0; i < numVals; i++) {
            if(parSorted[i] != allVals[i]) {
                printf("badSort %d\n", i);
                break;
            }
        } */
       print_numbers(argv[2], parSorted, numVals);
    }

    /* for (int i = 0; i < BASE_LENGTH; i++)
    {
        printf("rank: %d, result[%d] %d\n", rank, i, result[i]);
    } */
    MPI_Finalize();
}