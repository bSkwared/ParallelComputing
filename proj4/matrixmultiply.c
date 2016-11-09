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
#include <string.h>
#include <time.h>


#define DATA_MSG     0
#define PROMPT_MSG   1
#define RESPONSE_MSG 2


#define OPEN_FILE_ERROR -1
#define MALLOC_ERROR    -2
#define INVALID_MATRIX  -3


#define MIN(a,b) 	         ((a) < (b) ? (a) : (b))
#define BLOCK_LOW(id,p,n)    ((id)*(n)/(p))
#define BLOCK_HIGH(id,p,n)   (BLOCK_LOW((id)+1,p,n) - 1)
#define BLOCK_SIZE(id,p,n)   (BLOCK_LOW((id)+1,p,n) - BLOCK_LOW(id,p,n))
#define BLOCK_OWN(index,p,n) (((p)*((index)+1)-1)/(n))


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


// Exchanges rows up and down so everyone has what they need each iteration
void exchangeRows(char** matrix, int rank, int numProcs, int rows, int cols);


// Prints out a matrix that is rows x cols
void printSubmatrix(long long **subMatrix, int numRows, int numCols);


// Gets all row information from processes and prints the matrix
void printRowStripedMatrix(long long** matrix, int size, int rank, int numProcs); 


int main(int argc, char* argv[]) {

    double startTime; // Seconds at start of the program
    double seqToPar;  // Seconds at end of reading matrix from file
    double parToSeq;  // Seconds at end of loop
    double endTime;   // Seconds at end of program

    int myRank;       // Which number process I am [0, (n-1)]
    int numProcs;     // How many processes there are going to be

    int i;   // Used for iterating things
    int r;   // Used for iterating rows
    int c;   // Used for iteration columns
    int sum; // Used for summing somethings

    const int MAX_FILE_LEN = 256; // Maximum length of a filename
    char filename[MAX_FILE_LEN];  // Filename of matrix

    int*  aStorage;    // Bulk storage for my portion of the matrix
    int** aMatrix;     // 2D version of bulk storage                

    int*  bStorage;
    int** bMatrix;

    long long*  cStorage;
    long long** cMatrix;

    int matrixSize;
    int myRows;           // Dimensions of my matrix
    int myCols;

    int*  counterStorage; // Bulk storage for counting neighbors
    int** counter;        // 2D version of the above

    int printMod;         // Command line arguments for number of iterations
    int numIterations;    // and how frequenctly to print out the matrix



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


    // Allocate storage for neighbor counting
    cStorage = (long long*)  malloc(myRows * myCols * sizeof(long long));
    cMatrix  = (long long**) malloc(myRows * sizeof(long long*));

    // Exit if memory allocation failed
    if (cStorage == NULL || cMatrix == NULL) {
        MPI_Abort(MPI_COMM_WORLD, MALLOC_ERROR);
    }

    // Link C matrix
    cMatrix[0] = cStorage;
    for (i = 1; i < myRows; ++i) {
        cMatrix[i] = cMatrix[i-1] + myCols;
    }

    for (r = 0; r < myRows; ++r) {
        for (c = 0; c < myCols; ++c) {
            cMatrix[r][c] = aMatrix[r][c];
        }
    }

    // Print matrix once before modifying it
    printRowStripedMatrix(cMatrix, matrixSize, myRank, numProcs);

    // BEGIN parallel operations
    seqToPar = MPI_Wtime();


    // END parallel operatinos
    parToSeq = MPI_Wtime();

    // Print out the resulting matrix
    if (myRank == 0) {
        printf("\n\n");
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
       fprintf(stderr, "%d,%d,"/*%d,%d,%d,%.15f,%.15f,*/"%.15f\n", numProcs,
                     matrixSize, endTime-startTime);//seqToPar-startTime, parToSeq-startTime, endTime-startTime); 
    }

    MPI_Finalize();
    return 0;
}



void readRowStripedMatrices(char* filename, int*** aMatrix, int** aStorage,
                            int*** bMatrix, int** bStorage,  int* matSize,
                            int myRank, int numProcs) {

    int** myAMatrix;  // Dereferenced version of subMatrix
    int*  myAStorage; // Dereferenced version of bulkStorage

    int** myBMatrix;
    int*  myBStorage;

    int size;
    int myRows;  // How many rows a process has, used for distribution
    int myCols;

    FILE* matrixFile; // File pointer for matrix file
    int intsRead;    // Used with fread to see how much data was read

    int i;
    int r;
    int c;
    //char junk;        // Somewhere to toss newlines

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

    myAStorage = *aStorage;
    myAMatrix  = *aMatrix;
    myBStorage = *bStorage;
    myBMatrix  = *bMatrix;



    // Exit if memory allocation failed
    if (aStorage == NULL || aMatrix == NULL || bStorage == NULL || aMatrix == NULL) {
        MPI_Abort(MPI_COMM_WORLD, MALLOC_ERROR);
    }


    // Link up sub-matrtix
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

//                // Remove newline character
//                if (fscanf(matrixFile, "%c", &junk) != 1) {
//                    MPI_Abort(MPI_COMM_WORLD, OPEN_FILE_ERROR);
//                }
                
                // Read A row
                for (c = 0; c < myCols; ++c) {
                    intsRead += fscanf(matrixFile, "%d", myAMatrix[r]+c);
                }

                // Read B row
                for (c = 0; c < myCols; ++c) {
                    intsRead += fscanf(matrixFile, "%d", myBMatrix[r]+c);
                }
            }

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


void exchangeRows(char** matrix, int rank, int numProcs, int rows, int cols) {
    
    MPI_Status status;

    // If I am not process 0, send my bottom row to the process below me
    if (rank > 0) {
         MPI_Send(matrix[1], cols, MPI_CHAR, rank - 1, 
                     DATA_MSG, MPI_COMM_WORLD);
    }
 
    // If there is a process above me, get their bottom row and send them my top
    if (rank < numProcs - 1) {
         MPI_Recv(matrix[rows - 1], cols, MPI_CHAR, rank + 1,
                     DATA_MSG, MPI_COMM_WORLD, &status);
 
         MPI_Send(matrix[rows - 2], cols, MPI_CHAR, rank + 1, 
                     DATA_MSG, MPI_COMM_WORLD);
    }
    
    // If There is someone below me, get their top row
    if (rank > 0) {
         MPI_Recv(matrix[0], cols, MPI_CHAR, rank - 1,
                     DATA_MSG, MPI_COMM_WORLD, &status);
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










