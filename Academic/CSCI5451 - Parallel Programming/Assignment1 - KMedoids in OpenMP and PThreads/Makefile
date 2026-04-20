all: openmp pthreads

openmp: km_openmp.cpp 
    g++ -fopenmp -O3 -o km_openmp km_openmp.cpp

pthreads: km_pthreads.cpp
    g++ -pthread -O3 -o km_pthreads km_pthreads.cpp