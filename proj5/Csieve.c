//Sieve of Eratosthenes (Part C: Remove broadcast)
//******************************************************************************
// Csieve.c
//
// Summary: Each process will find primes < sqrt(n) on their own
//
// Authors: Spencer Pullins & Blake Lasky
// Created: 10/2/2016
//******************************************************************************

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define MIN(a,b)             ((a) < (b) ? (a) : (b))
#define BLOCK_LOW(id,p,n)    ((id)*(n)/(p))
#define BLOCK_HIGH(id,p,n)   (BLOCK_LOW((id)+1,p,n) - 1)
#define BLOCK_SIZE(id,p,n)   (BLOCK_LOW((id)+1,p,n) - BLOCK_LOW(id,p,n))
#define BLOCK_OWN(index,p,n) (((p)*((index)+1)-1)/(n))

#pragma parallelize
int main(int argc, char* argv[]){
    int myPrimes;       // Local  prime count
    int totalPrimes;    // Global prime count

    double startTime;   // Seconds from epoch to the start of the loop
    double endTime;     // Seconds from epoch to the end of the combine

    int myRank;         // Which number process I am [0, (n-1)]
    int numProcs;       // How many processes there are going to be

    int firstMultiple;  // Index of first multiple
    int myMax;          // Highest value on this proc
    int index;          // Index of current prime
    int myMin;          // Lowest value on this proc
    char *marked;       // Portioin of 2,...,n
    char *myMarked;     // Portiona from 2,...,sqrt(n)
    int *foundPrimes;   // Primes found < sqrt(n)
    int primeCounter;   // Number of primes < sqrt(n)
    int sqrtMax;        // sqrt(maxNum)
    int maxNum;         // Sieving from 2,...,n
    int p0Size;         // Size of proc 0's subarray
   // int prime;          // Current prime
    int mySize;         // Elements in marked
    int i;
    int j;
    int numThreads;


    // Make sure user provided number of iterations
    if (argc != 3) {
        printf("\nUsage: %s maxNum\n", argv[0]);
        return 1;
    }

    maxNum = atoi(argv[1]);
    if (maxNum <= 0) {
        printf("\nError: max number must be a positive integer\n");
        return 2;
    }

    numThreads = atoi(argv[2]);
    if (numThreads <= 0) {
        printf("\nError: max number must be a positive integer\n");
        return 2;
    }

    sqrtMax = (int) sqrt(maxNum);
    
    // Allocate this process's share of the array to zeros
    if (maxNum%2 ==0){
        maxNum /= 2;
        --maxNum;
    } else {
        maxNum/=2;
    }
    marked   = (char *) calloc(maxNum,  sizeof(char));
    myMarked = (char *) calloc(sqrtMax, sizeof(char));
    foundPrimes = (int *) malloc((sqrtMax/2) * sizeof(int));

    // Exit if we couldn't allocate memory
    if(marked == NULL || myMarked == NULL || foundPrimes == NULL){
        printf("Cannot allocate enough memory\n");
        return 1;
    }


    // Mark primes < sqrt(n)
    index = 0;
    primeCounter = 0;
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

    // Mark primes in my range
    #pragma omp parallel for num_threads(numThreads) private(j, prime, firstMultiple)
    for (i = 0; i < primeCounter; ++i) {
        prime = foundPrimes[i];
        firstMultiple = prime*prime/2 -1;
        // Mark primes
        for(j = firstMultiple; j < maxNum; j += prime){
            marked[j] = 1;
        }
    }


    // Count local primes found, include 2 if proc 0
    //if (myRank == 0) {
        myPrimes = 1;
    //} else {
    //    myPrimes = 0;
    //}
printf("\n\n\n");
    for(i = 0; i < maxNum; i++){
        if(marked[i] == 0){
            myPrimes++;
        }
    }

    // Total primes from all the processes
    printf("primes: %d\n", myPrimes);
    
    free(foundPrimes);
    free(myMarked);
    free(marked);
    return 0;
}








