// Matrix Multiplication
//******************************************************************************
// matrix3.c
//
// Summary: Multiply matrices of the same size in parallel with blocking.
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

#define THRESHOLD 99916

#define DATA_MSG     0
#define PROMPT_MSG   1
#define RESPONSE_MSG 2


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


// Exchanges blocks up and down so everyone has what they need each iteration
void exchangeBlocks(int* bStorage, int bSize, int* tStorage, int rank, int numProcs);


// Prints out a matrix that is numRows x numCols
void printSubmatrix(long long **subMatrix, int numRows, int numCols);


// Gets all row information from processes and prints the matrix
void printRowStripedMatrix(long long** matrix, int size, int rank, int numProcs); 


// Multiplies matrices with blocking
void matrixMultiply(int **a, int **b, long long **c, int crow, int ccol, 
                    int arow, int acol, int brow, int bcol, 
                    int L, int M, int N, int matSize);


int main(int argc, char* argv[]) {

    double startTime; // Seconds at start of the program
    double seqToPar;  // Seconds at end of reading matrix from file
    double parToSeq;  // Seconds at end of loop
    double endTime;   // Seconds at end of program

    int myRank;       // Which number process I am [0, (n-1)]
    int numProcs;     // How many processes there are going to be

    int i;   // Used for iterating things
    int j;   // Used for iterating things
    int r;   // Used for iterating rows
    int c;   // Used for iteration columns
    
    long long sum;    // Used for summing rows times columns

    const int MAX_FILE_LEN = 256; // Maximum length of a filename
    char filename[MAX_FILE_LEN];  // Filename of matrix

    int*  aStorage;   // Bulk storage for my portion of the A matrix
    int** aMatrix;    // 2D version of bulk storage                

    int*  bStorage;   // B matrix storage
    int** bMatrix;
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
    
    // Initialize cMatrix to 0's
    for (r = 0; r < myRows; ++r) {
        for (c = 0; c < myCols; ++c) {
            cMatrix[r][c] = 0;
        }
    }


    // Multiply matrices
    for (i = 0; i < numProcs; ++i) {
        matrixMultiply(aMatrix, bMatrix, cMatrix, 0, 0, 0, 
                        ((i+myRank)%numProcs)*myRows, 0, 0, myRows, myRows, 
                        myCols, matrixSize);

        // Don't exchange if I am not multiplying anymore
        if (i != numProcs-1) {
            exchangeBlocks(bStorage, bSize, tempStorage, myRank, numProcs);
        }
    }


    // Print resulting matrix
    printRowStripedMatrix(cMatrix, matrixSize, myRank, numProcs);


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


    // p-1 broadcast matrix data
    if (myRank == (numProcs - 1)) {

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

            // I don't need to send data to myself
            if (i != myRank) {
                MPI_Send(myAStorage, myRows * myCols, MPI_INT, i,
                            DATA_MSG, MPI_COMM_WORLD);

                MPI_Send(myBStorage, myRows * myCols, MPI_INT, i,
                            DATA_MSG, MPI_COMM_WORLD);
            }
        }

        close(matrixFile);

    } else {
        // Receive matrix data
        MPI_Recv(myAStorage, myRows * myCols, MPI_INT, numProcs - 1,
                    DATA_MSG, MPI_COMM_WORLD, &status);

        MPI_Recv(myBStorage, myRows * myCols, MPI_INT, numProcs - 1,
                    DATA_MSG, MPI_COMM_WORLD, &status);
    }
}


void exchangeBlocks(int* bStorage, int bSize, int* tStorage, int myRank, int numProcs) {
    
    MPI_Status status;
    MPI_Request sReq;
    MPI_Request rReq;

    int sendTo;   // Which processors to communicate with
    int recvFrom;

    int i; 

    // Calculate who to communicate with.
    recvFrom   = (myRank+1) % numProcs;
    sendTo = (myRank+numProcs-1) % numProcs;

    MPI_Isend(bStorage, bSize, MPI_INT, sendTo, DATA_MSG, MPI_COMM_WORLD, &sReq);
    MPI_Irecv(tStorage, bSize, MPI_INT, recvFrom, DATA_MSG, MPI_COMM_WORLD, &rReq);
    

    MPI_Wait(&sReq, &status);
    MPI_Wait(&rReq, &status);

    for (i = 0; i < bSize; ++i) {
        bStorage[i] = tStorage[i];
    }
}


void printRowStripedMatrix(long long** matrix, int size, int rank, int numProcs) {
    
    long long*  bulkStorage;    // Temporary storage for data from other processes
    long long** receivedMatrix;

    int i;

    int maxRows;
    int myLow;
    int nextLow;
    

    MPI_Status status;
    int prompt;

    int rowsPerProc = size/numProcs;
    int numCols     = size;


    if (rank == 0) {
        // Print my submatrix
        printSubmatrix(matrix, rowsPerProc, numCols);

        // Print everyone elses
        if (numProcs > 1) {

            // Allocate and storage and hookup 2D matrix
            bulkStorage    = (long long*) malloc(rowsPerProc * numCols 
                                                    * sizeof(long long));

            receivedMatrix = (long long**) malloc(rowsPerProc * sizeof(long long*));

            if (bulkStorage == NULL || receivedMatrix == NULL) {
                MPI_Abort(MPI_COMM_WORLD, MALLOC_ERROR);
            }

            receivedMatrix[0] = bulkStorage;
            for (i = 1; i < rowsPerProc; ++i) {
                receivedMatrix[i] = receivedMatrix[i-1] + numCols;
            }

            
            // Receive matrices from everyone else and print them out
            for (i = 1; i < numProcs; ++i) {
                
                // Request matrix from proc i
                MPI_Send(&prompt, 1, MPI_INT, i, PROMPT_MSG, MPI_COMM_WORLD);

                // Get matrix from proc i
                MPI_Recv(bulkStorage, rowsPerProc*numCols, MPI_LONG_LONG, i, 
                            RESPONSE_MSG, MPI_COMM_WORLD, &status);

                // Print matrix from proc i
                printSubmatrix(receivedMatrix, rowsPerProc, numCols);
            }
    
            free(receivedMatrix);
            free(bulkStorage);
        }

    } else {
        // If I am not process 0, send my submatrix to him
        MPI_Recv(&prompt, 1, MPI_INT, 0, PROMPT_MSG, MPI_COMM_WORLD, &status);
        MPI_Send(*matrix, rowsPerProc*numCols, MPI_LONG_LONG, 0, 
                    RESPONSE_MSG, MPI_COMM_WORLD);
    }
}


void printSubmatrix(long long **subMatrix, int numRows, int numCols) {
    int r;
    int c;

    for (r = 0; r < numRows; ++r) {
        for (c = 0; c < numCols; ++c) {
            printf("%lld ", subMatrix[r][c]);
       }
       printf("\n");
    }
}


void matrixMultiply(int **a, int **b, long long **c, int crow, int ccol, 
                    int arow, int acol, int brow, int bcol, 
                    int L, int M, int N, int matSize) {

    int lhalf[3], mhalf[3], nhalf[3]; // used for splitting matrix
    int i, j, k;
    int *aptr;
    int *bptr;
    long long sum;

    // Make recursive call if matrices will not fit in cache
    if (M*N > THRESHOLD) {
        lhalf[0] = 0; lhalf[1] = L/2; lhalf[2] = L-L/2;
        mhalf[0] = 0; mhalf[1] = M/2; mhalf[2] = M-M/2;
        nhalf[0] = 0; nhalf[1] = N/2; nhalf[2] = N-N/2;

        for (i = 0; i < 2; ++i) {
            for (j = 0; j < 2; ++j) {
                for (k = 0; k < 2; ++k) {
                    matrixMultiply(a, b, c,
                                   crow + lhalf[i], ccol + nhalf[j],
                                   arow + lhalf[i], acol + mhalf[k],
                                   brow + mhalf[k], bcol + nhalf[j],
                                   lhalf[i+1], mhalf[k+1], nhalf[j+1], matSize);
                }
            }
        }

    // Multiply if they will fit in the cache
    } else {

        for (i = 0; i < L; ++i) {
            for (j = 0; j < N; ++j) {
                aptr = &(a[arow+i][acol]);
                bptr = &(b[brow][bcol+j]);
                sum = 0;
                for (k = 0; k < M; ++k) {
                    sum += *(aptr++) * (*bptr);
                    bptr += matSize;
                }
                c[crow+i][ccol+j] += sum;
            }
        }
    }
}
