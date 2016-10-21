// Game of Life
//******************************************************************************
// life.cpp
//
// Summary: Simulation of Conway's game of Life. Cells live and die.
//
// Authors: Spencer Pullins & Blake Lasky
// Created: Oct 2016
//******************************************************************************

#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>


#define OPEN_FILE_ERROR -1

#define MIN(a,b) 	         ((a) < (b) ? (a) : (b))
#define BLOCK_LOW(id,p,n)    ((id)*(n)/(p))
#define BLOCK_HIGH(id,p,n)   (BLOCK_LOW((id)+1,p,n) - 1)
#define BLOCK_SIZE(id,p,n)   (BLOCK_LOW((id)+1,p,n) - BLOCK_LOW(id,p,n))
#define BLOCK_OWN(index,p,n) (((p)*((index)+1)-1)/(n))

struct dimensions {
    int numRows;
    int numCols;
};
typedef struct dimensions Dimensions;

void readRowStripedMatrix(
    char*       filename,    // Name of file with matrix
    char***     subMatrixs,  // Pointer to submatrix
    char**      bulkStorage, // Bulk storage of sub matrix
    Dimensions* dimension,   // Rows and cols in submatrix
    int         myRank,
    int         numProcs);


int main(int argc, char* argv[]) {

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
	int prime;          // Current prime
	int mySize;         // Elements in marked
    int i;
    int j;



    char filename[100] = "matrix.in";
    char*** matrix;
    char** bulkStorage;
    Dimensions d;



    /*// Make sure user provided number of iterations
    if (argc != 2) {
        printf("\nUsage: %s maxNum\n", argv[0]);
        return 1;
    }

    maxNum = atoi(argv[1]);
    if (maxNum <= 0) {
        printf("\nError: max number must be a positive integer\n");
        return 2;
    }*/

	// Begin MPI
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
	MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

    readRowStripedMatrix(filename, matrix, bulkStorage, &d, myRank, numProcs);

    /*// Start timing
	MPI_Barrier(MPI_COMM_WORLD);
	startTime = MPI_Wtime();
	

	// Figure out this process's share of the array, as well as the integers
    // represented by the first and last array elements
	myMin  = 2 + BLOCK_LOW(myRank, numProcs, maxNum-1);
    if (myMin % 2 == 0) {
        ++myMin;
    }

	myMax  = 2 + BLOCK_HIGH(myRank, numProcs, maxNum-1);
    if (myMax % 2 == 0) {
        --myMax;
    }

	mySize = (myMax - myMin)/2 + 1;
    sqrtMax = (int) sqrt(maxNum);

    //printf("low: %d\nhigh: %d\nsize: %d\n", myMin, myMax, mySize);

	
    // Bail out if all the primes used for sieving are not all held by process 0
	p0Size = (maxNum - 1) / numProcs;
    //printf("size: %d,  procsz: %d\n", mySize, p0Size);
	if (p0Size < sqrtMax){
		if(myRank == 0){
			printf("Too many processes\n");
		}
		MPI_Finalize();
		return 1;
	}


	// Allocate this process's share of the array to zeros
    marked   = (char *) calloc(mySize,  sizeof(char));
    myMarked = (char *) calloc(sqrtMax, sizeof(char));
    foundPrimes = (int *) malloc((sqrtMax/2) * sizeof(int));

    // Exit if we couldn't allocate memory
	if(marked == NULL || myMarked == NULL || foundPrimes == NULL){
		printf("Cannot allocate enough memory\n");
		MPI_Finalize();
		return 1;
	}


    // Mark primes < sqrt(n)
    index = 0;
    primeCounter = 0;
	prime = 3;
    while (prime < sqrtMax) {
        foundPrimes[primeCounter] = prime;
        ++primeCounter;

        for (j = (prime*prime - 3)/2; j < sqrtMax; j += prime) {
            myMarked[j] = 1;
        }

        while(myMarked[++index] == 1);
        prime = index*2 + 3;
    }

    // Mark primes in my range
    index = 1;
    prime = 3;
    for (i = 0; i < primeCounter; ++i) {
        prime = foundPrimes[i];

        // Find first multiple of prime in my marked array
        if (prime * prime > myMin){
			firstMultiple = (prime * prime - myMin) / 2;
		} else if (myMin % prime == 0){
            firstMultiple = 0;
        } else {
            firstMultiple = ((myMin % prime) * (prime/2)) % prime;
        }

        // Mark primes
		for(j = firstMultiple; j < mySize; j += prime){
			marked[j] = 1;
		}
    }


    // Count local primes found, include 2 if proc 0
	if (myRank == 0) {
        myPrimes = 1;
    } else {
        myPrimes = 0;
    }

	for(i = 0; i < mySize; i++){
		if(marked[i] == 0){
			myPrimes++;
		}
	}
    //printf("myPrimes: %d\n", myPrimes);

    // Total primes from all the processes
	MPI_Reduce(&myPrimes, &totalPrimes, 
                1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	
    endTime = MPI_Wtime();


	// Print the results
	if(myRank == 0){
		printf("C,%d,%d,%.15f\n", numProcs, maxNum, endTime - startTime);
	}

    */
	MPI_Finalize();
	return 0;
}




void readRowStripedMatrix(char* filename, char*** subMatrixs, char** bulkStorage,
                          Dimensions* dimension, int myRank, int numProcs) {

    int* numRows;
    int* numCols;

    FILE* matrixFile;

    int numRead; 

    
    numRows = &(dimension->numRows);
    numCols = &(dimension->numCols);

    if (myRank == (numProcs - 1)) {
        matrixFile = fopen(filename, "r");
        
        if (matrixFile == NULL 
            || fscanf(matrixFile, "%d %d", numRows, numCols) != 2) {
            MPI_Abort(MPI_COMM_WORLD, OPEN_FILE_ERROR);

        }
    }

    MPI_Bcast(dimension, sizeof(Dimensions), MPI_INT, numProcs-1, MPI_COMM_WORLD);

    if (*numRows == 0) {
        MPI_Abort(MPI_COMM_WORLD, OPEN_FILE_ERROR);
    }

    printf("rows: %d\tcols: %d\n", *numRows, *numCols);

}




















