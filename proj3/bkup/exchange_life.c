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
#include <time.h>


#define DATA_MSG     0
#define PROMPT_MSG   1
#define RESPONSE_MSG 2


#define OPEN_FILE_ERROR -1
#define MALLOC_ERROR    -2


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
    char***     subMatrix,   // Pointer to submatrix
    char**      bulkStorage, // Bulk storage of sub matrix
    Dimensions* dimension,   // Rows and cols in submatrix
    int         myRank,
    int         numProcs);

void exchangeRows(char** matrix, int rank, int numProcs, int rows, int cols);

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
    char** matrix;
    char* bulkStorage;
    Dimensions d;
    int myRows;
    int myCols;
    MPI_Status status;



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

    readRowStripedMatrix(filename, &matrix, &bulkStorage, &d, myRank, numProcs);
    sleep(myRank);


    myRows = BLOCK_SIZE(myRank, numProcs, d.numRows) + 2;
    myCols = d.numCols + 2;

    printf("myRank: %d   r#:%d   c#: %d\n", myRank, myRows, myCols);
    for (j = 0; j < myRows; ++j) {
        for (i = 0; i < myCols; ++i) {
            printf("%c", matrix[j][i]);
        }
        printf("  %d:%d\n", i, j);
    }

    exchangeRows(matrix, myRank, numProcs, myRows, myCols);

sleep(myRank);    
    printf("\n\nmyRank : %d\n", myRank);
    for (j = 0; j < myRows; ++j) {
        for (i = 0; i < myCols; ++i) {
            printf("%c", matrix[j][i]);
        }
        printf("  \n");
    }

    free(bulkStorage);
    free(matrix);

	MPI_Finalize();
	return 0;
}




void readRowStripedMatrix(char* filename, char*** subMatrix, char** bulkStorage,
                          Dimensions* dimension, int myRank, int numProcs) {

    char** myMatrix;
    char*  myStorage;

    int* numRows;
    int* numCols;
    int myRows;
    int myCols;

    FILE* matrixFile;

    int numRead; 
    int i;
    int index;
    int bytesRead;

    int myLow;
    int size;
    int nextLow;

    int r;
    int c;
    char junk;

    MPI_Status status;

    
    numRows = &(dimension->numRows);
    numCols = &(dimension->numCols);

    // Read in matrix dimensions
    if (myRank == (numProcs - 1)) {
        matrixFile = fopen(filename, "r");
        
        if (matrixFile == NULL 
            || fscanf(matrixFile, "%d %d", numRows, numCols) != 2) {

            MPI_Abort(MPI_COMM_WORLD, OPEN_FILE_ERROR);
        }
    }

    // Send dimensions to every process
    MPI_Bcast(dimension, sizeof(Dimensions), MPI_INT, numProcs-1, MPI_COMM_WORLD);
    if (*numRows == 0) {
        MPI_Abort(MPI_COMM_WORLD, OPEN_FILE_ERROR);
    }


    // Allocate storage 
    myRows = BLOCK_SIZE(myRank, numProcs, *numRows) + 2;
    myCols = *numCols + 2;

    *bulkStorage = (char*)  malloc(myRows * myCols * sizeof(char));
    *subMatrix   = (char**) malloc(myRows * sizeof(char*));

    myMatrix  = *subMatrix;
    myStorage = *bulkStorage;


    // Exit if memory allocation failed
    if (bulkStorage == NULL || subMatrix == NULL) {
        MPI_Abort(MPI_COMM_WORLD, MALLOC_ERROR);
    }


    // Link up sub-matrtix
    index = 0;
    for (i = 0; i < myRows; ++i) {
        myMatrix[i] = myStorage + index;
        index += myCols;
    }


    // Broadcast matrix data
    if (myRank == (numProcs - 1)) {

        myLow = 0;
        for (i = 0; i < numProcs; ++i) {
            // Use nextLow to avoid redundent computation
            nextLow = BLOCK_LOW(i+1, numProcs, *numRows);
            size = nextLow - myLow;

            // Setup myLow for next iteration
            myLow = nextLow;
                
            // Fill top and bottom row with 0's
            for (c = 0; c < myCols; ++c) {
                myMatrix[0][c] = '0';
                myMatrix[size + 1][c] = '0';
            }
        
            // Read in rows
            bytesRead = 0;
            for (r = 0; r < size; ++r) {

                // Remove newline character
                if (fscanf(matrixFile, "%c", &junk) != 1) {
                    MPI_Abort(MPI_COMM_WORLD, OPEN_FILE_ERROR);
                }
                
                // Read row and add 0's to front and back of row
                bytesRead += fread(myMatrix[r+1] + 1, sizeof(char),
                                    *numCols, matrixFile);

                myMatrix[r+1][0] = '0';
                myMatrix[r+1][myCols - 1] = '0';
            }

            if (bytesRead != size * (*numCols)) {
                MPI_Abort(MPI_COMM_WORLD, OPEN_FILE_ERROR);
            }


            // I don't need to send data to myself
            if (i != myRank) {
                MPI_Send(myStorage, (size + 2) * myCols, MPI_CHAR, i,
                            DATA_MSG, MPI_COMM_WORLD);
            }
        }

        close(matrixFile);

    } else {
        // Receive matrix data
        MPI_Recv(myStorage, myRows * myCols, MPI_CHAR, numProcs - 1,
                    DATA_MSG, MPI_COMM_WORLD, &status);
    }
}


void exchangeRows(char** matrix, int rank, int numProcs, int rows, int cols){
    
    MPI_Status status;

    if (rank > 0) {
         MPI_Send(matrix[1], cols, MPI_CHAR, rank - 1, 
                     DATA_MSG, MPI_COMM_WORLD);
    }
 
    if (rank < numProcs - 1) {
         MPI_Recv(matrix[rows - 1], cols, MPI_CHAR, rank + 1,
                     DATA_MSG, MPI_COMM_WORLD, &status);
 
         MPI_Send(matrix[rows - 2], cols, MPI_CHAR, rank + 1, 
                     DATA_MSG, MPI_COMM_WORLD);
    }
 
    if (rank > 0) {
         MPI_Recv(matrix[0], cols, MPI_CHAR, rank - 1,
                     DATA_MSG, MPI_COMM_WORLD, &status);
    }
}


















