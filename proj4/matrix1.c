// Matrix Multiplication
//******************************************************************************
// matrix1.c
//
// Summary: Multiply matrices of the same size without blocking.
//
// Authors: Spencer Pullins & Blake Lasky
// Created: Nov 2016
//******************************************************************************

#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define OPEN_FILE_ERROR -1
#define MALLOC_ERROR    -2
#define INVALID_MATRIX  -3

// Reads a matrix from a file and sends the blocks to coreesponding processes
void readRowStripedMatrices(
    char*  filename, // Name of file with matrices
    int*** aMatrix,  // Bulk storage of A matrix
    int**  aStorage, // 2D A matrix
    int*** bMatrix,  // Bulk storage of B matrix
    int**  bStorage, // 2D B matrix
    int*   matSize,  // width/height of square matrix
    int    myRank,
    int    numProcs);


int main(int argc, char* argv[]) {

    double startTime; // Seconds at start of the program
    double endTime;   // Seconds at end of program

    int myRank;       // Which number process I am [0, (n-1)]
    int numProcs;     // How many processes there are going to be

    int i;   // Used for iterating things
    int j;   // Used for iterating things
    int k;   // Used for iterating things
    int r;   // Used for iterating rows
    int c;   // Used for iteration columns
    
    long long sum;    // Used for summing rows times columns

    const int MAX_FILE_LEN = 256; // Maximum length of a filename
    char filename[MAX_FILE_LEN];  // Filename of matrix

    int*  aStorage;   // Bulk storage for my portion of the A matrix
    int** aMatrix;    // 2D version of bulk storage    
    int*  aptr;       // Used for multiplication

    int*  bStorage;   // B matrix storage
    int** bMatrix;
    int*  bptr;       // Used for multiplication
    int   bSize;      // Number of elements in bStorage

    int* tempStorage; // Used for data transfer

    long long*  cStorage; // C matrix bulk storage
    long long** cMatrix;  // 2D C matrix

    int matrixSize;   // Width of square matrix
    int myRows;       // Dimensions of my matrix
    int myCols;


    // Check command line arguments
    if (argc != 2) {
        printf("\nUsage: %s filename\n", argv[0]);
        return 1;
    }
    strncpy(filename, argv[1], sizeof(filename)); 


	// Begin MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

    startTime = MPI_Wtime();


    // Read the matrix in from file and get my portion of it
    readRowStripedMatrices(filename, &aMatrix, &aStorage, &bMatrix, &bStorage,
                            &matrixSize, myRank, numProcs);


    // How big will my portion of the matrix be?
    myRows = matrixSize / numProcs;
    myCols = matrixSize;
    bSize  = myRows * myCols;


    // Allocate storage for C, result, matrix
    cStorage = (long long*)  malloc(myRows * myCols * sizeof(long long));
    cMatrix  = (long long**) malloc(myRows * sizeof(long long*));

    tempStorage = (int*) malloc(myRows * myCols * sizeof(int));


    // Exit if memory allocation failed
    if (cStorage == NULL || cMatrix == NULL || tempStorage == NULL) {
        MPI_Abort(MPI_COMM_WORLD, MALLOC_ERROR);
    }

    // Link up C matrix
    cMatrix[0] = cStorage;
    for (i = 1; i < myRows; ++i) {
        cMatrix[i] = cMatrix[i-1] + myCols;
    }
    

    // Multiply matrices
    for (i = 0; i < matrixSize; ++i) {
        for (j = 0; j < matrixSize; ++j) {
            aptr = &(aMatrix[i][0]);
            bptr = &(bMatrix[0][j]);
            sum = 0;
            for (k = 0; k < matrixSize; ++k) {
                sum += *(aptr++) * (*bptr);
                bptr += matrixSize;
            }
            cMatrix[i][j] = sum;
        }
    }

    
    // Print resulting matrix
    for (r = 0; r < myRows; ++r) {
        for (c = 0; c < myCols; ++c) {
            printf("%lld ", cMatrix[r][c]);
        }
        printf("\n");
    }

    
    // Free dynami memory
    free(aStorage);
    free(aMatrix);

    free(bStorage);
    free(bMatrix);
    
    free(cStorage);
    free(cMatrix);


    // Print runtimes to stderr so stdout can be piped to /dev/null
    endTime = MPI_Wtime();
    if (myRank == 0) {
       fprintf(stderr, "%d,%d,%.15f\n", numProcs, matrixSize, endTime-startTime);
    }

    MPI_Finalize();
    return 0;
}



void readRowStripedMatrices(char* filename, int*** aMatrix, int** aStorage,
                            int*** bMatrix, int** bStorage,  int* matSize,
                            int myRank, int numProcs) {

    int** myAMatrix;  // Dereferenced version of aMatrix
    int*  myAStorage; // Dereferenced version of aStorage

    int** myBMatrix;  // Derefenced versions
    int*  myBStorage;

    int size;         // Width of the matrices
    int myRows;       // How many rows a process has, used for distribution
    int myCols;

    FILE* matrixFile; // File pointer for matrix file
    int intsRead;     // Used with fread to see how much data was read

    int i; // Iteration variables
    int r;
    int c;

    MPI_Status status;

    // Read in matrix dimensions
    if (myRank == (numProcs - 1)) {
        matrixFile = fopen(filename, "r");

        if (matrixFile == NULL 
            || fscanf(matrixFile, "%d", &size) != 1) {

            MPI_Abort(MPI_COMM_WORLD, OPEN_FILE_ERROR);
        }
    }

    // Send dimensions to every process
    MPI_Bcast(&size, 1, MPI_INT, numProcs-1, MPI_COMM_WORLD);
    if (size == 0 || size % numProcs != 0) {
        MPI_Abort(MPI_COMM_WORLD, INVALID_MATRIX);
    }
    *matSize = size;

    // Allocate storage 
    myRows = size/numProcs;
    myCols = size;

    *aStorage = (int*)  malloc(myRows * myCols * sizeof(int));
    *aMatrix  = (int**) malloc(myRows * sizeof(int*));

    *bStorage = (int*)  malloc(myRows * myCols * sizeof(int));
    *bMatrix  = (int**) malloc(myRows * sizeof(int*));

    // Copy pointers locally
    myAStorage = *aStorage;
    myAMatrix  = *aMatrix;
    myBStorage = *bStorage;
    myBMatrix  = *bMatrix;


    // Exit if memory allocation failed
    if (aStorage == NULL || aMatrix == NULL || bStorage == NULL || bMatrix == NULL) {
        MPI_Abort(MPI_COMM_WORLD, MALLOC_ERROR);
    }


    // Link up 2D versions of A and B
    myAMatrix[0] = myAStorage;
    myBMatrix[0] = myBStorage;
    for (i = 1; i < myRows; ++i) {
        myAMatrix[i] = myAMatrix[i-1] + myCols;
        myBMatrix[i] = myBMatrix[i-1] + myCols;
    }

    for (i = 0; i < numProcs; ++i) {
        // Read in rows
        intsRead = 0;
        for (r = 0; r < myRows; ++r) {
            
            // Read A row
            for (c = 0; c < myCols; ++c) {
                intsRead += fscanf(matrixFile, "%d", myAMatrix[r]+c);
            }

            // Read B row
            for (c = 0; c < myCols; ++c) {
                intsRead += fscanf(matrixFile, "%d", myBMatrix[r]+c);
            }
        }

        // Make sure we read in the correct number of values
        if (intsRead != 2 * myRows * myCols) {
            MPI_Abort(MPI_COMM_WORLD, OPEN_FILE_ERROR);
        }
    }

    close(matrixFile);
}

