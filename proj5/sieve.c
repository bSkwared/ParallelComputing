//Sieve of Eratosthenes (Part T: Multithreaded)
//******************************************************************************
// sieve.c
//
// Summary: Each process will find primes < sqrt(n) on their own
//
// Authors: Spencer Pullins & Blake Lasky
// Created: 12/2/2016
//******************************************************************************

#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(int argc, char* argv[]){

    double startTime;   // Seconds from epoch to the start of the loop
    double endTime;     // Seconds from epoch to the end of the combine

    char *marked;       // Portioin of 2,...,n
    char *myMarked;     // Portiona from 2,...,sqrt(n)
    int *foundPrimes;   // Primes found < sqrt(n)
    int j;
    

    struct timeval tm;
    gettimeofday(&tm, NULL);

    // Make sure user provided number of iterations
    if (argc != 3) {
        printf("\nUsage: %s maxNum\n", argv[0]);
        return 1;
    }

    int maxNum = atoi(argv[1]);
    int sqrtMax = (int) sqrt(maxNum);
    int originalMaxNum = maxNum;
    if (maxNum <= 0) {
        printf("\nError: max number must be a positive integer\n");
        return 2;
    }

    int numThreads = atoi(argv[2]);
    if (numThreads <= 0) {
        printf("\nError: max number must be a positive integer\n");
        return 2;
    }
    omp_set_num_threads(numThreads);

    
    // Allocate this process's share of the array to zeros
    --maxNum;
    maxNum /= 2;
    marked   = (char *) calloc(maxNum,  sizeof(char));
    myMarked = (char *) calloc(sqrtMax, sizeof(char));
    foundPrimes = (int *) malloc((sqrtMax/2) * sizeof(int));


    // Exit if we couldn't allocate memory
    if(marked == NULL || myMarked == NULL || foundPrimes == NULL){
        printf("Cannot allocate enough memory\n");
        return 1;
    }


    // Mark primes < sqrt(n)
    int index = 0;
    int primeCounter = 0;
    int prime = 3;
    while (prime <= sqrtMax) {
        foundPrimes[primeCounter] = prime;
        ++primeCounter;

        for (int j = (prime*prime - 3)/2; j < sqrtMax; j += prime) {
            myMarked[j] = 1;
        }

        ++index;
        while(myMarked[index] == 1) {
            ++index;
        }
        prime = index*2 + 3;
    }


    // Mark primes 
    int firstMultiple;
    #pragma omp parallel for private(j, prime, firstMultiple)
    for (int i = 0; i < primeCounter; ++i) {
        prime = foundPrimes[i];
        firstMultiple = prime*prime/2 -1;
        // Mark primes
        for(j = firstMultiple; j < maxNum; j += prime){
            marked[j] = 1;
        }
    }


    // Total primes from
    int totalPrimes = 1;
    for(int i = 0; i < maxNum; i++){
        if(marked[i] == 0){
            totalPrimes++;
        }
    }


    // Get time at end
    struct timeval end;
    gettimeofday(&end, NULL);

    // Runtime = end-start
    double runTime = (double)(end.tv_sec) + ((double) (end.tv_usec))/1.0e6;
    runTime -= (double)(tm.tv_sec) + ((double) (tm.tv_usec))/1.0e6;
   
    printf("Number of primes found: %d\n", totalPrimes);

    fprintf(stderr, "%d,%d,%.15f\n", numThreads, originalMaxNum, runTime);
    
    free(foundPrimes);
    free(myMarked);
    free(marked);
    return 0;
}








