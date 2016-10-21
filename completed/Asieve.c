//Sieve of Eratosthenes (Part A: Basic)
//******************************************************************************
// fileName
//
// Summary: Mainly copied from the book, slight cleanup done
//
// Authors: Spencer Pullins & Blake Lasky
// Created: 10/2/2016
//******************************************************************************

#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define MIN(a,b) 	         ((a) < (b) ? (a) : (b))
#define BLOCK_LOW(id,p,n)    ((id)*(n)/(p))
#define BLOCK_HIGH(id,p,n)   (BLOCK_LOW((id)+1,p,n) - 1)
#define BLOCK_SIZE(id,p,n)   (BLOCK_LOW((id)+1,p,n) - BLOCK_LOW(id,p,n))
#define BLOCK_OWN(index,p,n) (((p)*((index)+1)-1)/(n))

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
	int maxNum;         // Sieving from 2,...,n
	int p0Size;         // Size of proc 0's subarray
	int prime;          // Current prime
	int mySize;         // Elements in marked
    int i;




    // Make sure user provided number of iterations
    if (argc != 2) {
        printf("\nUsage: %s maxNum\n", argv[0]);
        return 1;
    }

    maxNum = atoi(argv[1]);
    if (maxNum <= 0) {
        printf("\nError: max number must be a positive integer\n");
        return 2;
    }

	// Begin MPI
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
	MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

    // Start timing
	MPI_Barrier(MPI_COMM_WORLD);
	startTime = MPI_Wtime();
	

	// Figure out this process's share of the array, as well as the integers
    // represented by the first and last array elements
	myMin = 2 + BLOCK_LOW(myRank, numProcs, maxNum-1);
	myMax = 2 + BLOCK_HIGH(myRank, numProcs, maxNum-1);
	mySize = BLOCK_LOW(myRank+1, numProcs, maxNum-1) - BLOCK_LOW(myRank, numProcs, maxNum-1);

    printf("low: %d\nhigh: %d\nsize: %d\n", myMin, myMax, mySize);

	
    // Bail out if all the primes used for sieving are not all held by process 0
	p0Size = (maxNum - 1) / numProcs;
    printf("size: %d,  procsz: %d\n", mySize, p0Size);
	if((2 + p0Size) < (int) sqrt(maxNum)){
		if(myRank == 0){
			printf("Too many processes\n");
		}
		MPI_Finalize();
		return 1;
	}


	// Allocate this process's share of the array.
    marked = (char *) malloc (mySize*sizeof(char));

    // Exit if we couldn't allocate memory
	if(marked == NULL){
		printf("Cannot allocate enough memory\n");
		MPI_Finalize();
		return 1;
	}
    
    // Initialize array to zero
	for(i = 0; i < mySize; i++){
		marked[i] = 0;
	}


    // Start with the first prime (2)
	if(myRank == 0){
		index = 0;
	}
	prime = 2;
	
	while (prime * prime <= maxNum) {
        // Find first multiple of prime in my marked array
        if (prime * prime > myMin){
			firstMultiple = prime * prime - myMin;
		} else if (myMin % prime == 0){
            firstMultiple = 0;
        } else {
            firstMultiple = prime - (myMin % prime);
        }

        // Mark primes
		for(i = firstMultiple; i < mySize; i+= prime){
			marked[i] = 1;
		}
        
        // Find and broadcast the next prime
		if(myRank == 0){
			while (marked[++index]);
			prime = index + 2;
		}
        MPI_Bcast (&prime, 1, MPI_INT, 0, MPI_COMM_WORLD);
	}


    // Count local primes found
	myPrimes = 0;
	for(i = 0; i < mySize; i++){
		if(marked[i] == 0){
			myPrimes++;
		}
	}
    printf("myPrimes: %d\n", myPrimes);

    // Total primes from all the processes
	MPI_Reduce(&myPrimes, &totalPrimes, 
                1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	
    endTime = MPI_Wtime();


	// Print the results
	if(myRank == 0){
		printf("%d primes are less than or equal to %d\n", totalPrimes, maxNum);
		printf("Total elapsed time: %10.6f\n", endTime - startTime);
	}

    
    free(marked);
	MPI_Finalize();
	return 0;
}








