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


#define MIN(a,b) 	         ((a) < (b) ? (a) : (b))
#define BLOCK_LOW(id,p,n)    ((id)*(n)/(p))
#define BLOCK_HIGH(id,p,n)   (BLOCK_LOW((id)+1,p,n) - 1)
#define BLOCK_SIZE(id,p,n)   (BLOCK_LOW((id)+1,p,n) - BLOCK_LOW(id,p,n))
#define BLOCK_OWN(index,p,n) (((p)*((index)+1)-1)/(n))


// Used for storing the number of rows and columns in the matrix
struct dimensions {
    int aRows;
    int inner;
    int bCols;
};
typedef struct dimensions Dimensions;


// Reads a matrix from a file and sends the blocks to coreesponding processes
void readRowStripedMatrix(
    char*       filename,  // Name of file with matrices
    int***      aMatrix,   // Pointer to submatrix
    int**       aStorage,  // Bulk storage of A matrix
    int***      bMatrix,   // 2D A matrix
    int**       bStorage,  // Bulk storage of B matrix
    Dimensions* dimension, // 2D B matrix
    int         myRank,
    int         numProcs);


// Exchanges rows up and down so everyone has what they need each iteration
void exchangeRows(char** matrix, int rank, int numProcs, int rows, int cols);


// Prints out a matrix that is rows x cols
void printSubmatrix(char **subMatrix, int rows, int cols);


// Gets all row information from processes and prints the matrix
void printRowStripedMatrix(char** subMatrix, int numRows, 
                            int myRows, int myCols, int myRank, int numProcs);

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

    int*  cStorage;
    int** cMatrix;

    Dimensions d;         // Dimensions of global matrix
    int myRows;           // Dimensions of my matrix
    int myCols;

    int*  counterStorage; // Bulk storage for counting neighbors
    int** counter;        // 2D version of the above

    int printMod;         // Command line arguments for number of iterations
    int numIterations;    // and how frequenctly to print out the matrix



    // Check command line arguments
    if (argc != 4) {
        printf("\nUsage: %s filename iterations printFrequency\n", argv[0]);
        return 1;
    }

    // Parse command line arguments
    numIterations = atoi(argv[2]);
    if (numIterations <= 0) {
        printf("\nError: number of iterations must be a positive integer");
        return 3;
    }

    printMod = atoi(argv[3]);
    if (printMod < 0) {
        printf("\nError: print frequency cannot be negative\n\n");
        return 4;
    }

    strncpy(filename, argv[1], sizeof(filename)); 


	// Begin MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

    startTime = MPI_Wtime();


    // Read the matrix in from file and get my portion of it
    readRowStripedMatrix(filename, &matrix, &bulkStorage, &d, myRank, numProcs);


    // How big will my portion of the matrix be?
    myRows = BLOCK_SIZE(myRank, numProcs, d.numRows);
    myCols = d.numCols;


    // Allocate storage for neighbor counting
    counterStorage = (int*)  malloc((myRows)*(myCols)*sizeof(int));
    counter        = (int**) malloc((myRows)*sizeof(int*));

    // Exit if memory allocation failed
    if (counterStorage == NULL || counter == NULL) {
        MPI_Abort(MPI_COMM_WORLD, MALLOC_ERROR);
    }

    // Link up sub-matrtix
    counter[0] = counterStorage;
    for (i = 1; i < myRows-2; ++i) {
        counter[i] = counter[i-1] + (myCols-2);
    }


    // Print matrix once before modifying it
    printRowStripedMatrix(matrix, d.numRows, myRows, myCols, myRank, numProcs);

    // BEGIN parallel operations
    seqToPar = MPI_Wtime();

    for (i = 0; i < numIterations; ++i) {
        // Get the top row from below and bottom row from above
        exchangeRows(matrix, myRank, numProcs, myRows, myCols);

        // Count how many neighbors everyone has
        for (r = 1; r < myRows-1; ++r) {
            sum = 0;
            for (c = 1; c < myCols - 1; ++c) {
                sum =  matrix[r-1][c-1];
                sum += matrix[r-1][c];
                sum += matrix[r-1][c+1];

                sum += matrix[r][c-1];
                sum += matrix[r][c+1];

                sum += matrix[r+1][c-1];
                sum += matrix[r+1][c];
                sum += matrix[r+1][c+1];

                counter[r-1][c-1] = sum;
            }
        }

        // Execute/resurrect based on number of neighbors
        for (r = 1; r < myRows-1; ++r) {
            for (c = 1; c < myCols - 1; ++c) {
                switch (counter[r-1][c-1]) {
                    case 2:
                        break;
                    case 3:
                        matrix[r][c] = 1;
                        break;
                    default:
                        matrix[r][c] = 0;
                        break;
                }
            }
        }


        // Print out the matrix
        if (printMod != 0 && (i % printMod) == printMod-1) {
            if (myRank == 0) {
                printf("\n\n");
            }
            printRowStripedMatrix(matrix, d.numRows, myRows, myCols, 
                                    myRank, numProcs);
        }
    }

    // END parallel operatinos
    parToSeq = MPI_Wtime();

    // Print out the resulting matrix
    if (myRank == 0) {
        printf("\n\n");
    }
    printRowStripedMatrix(matrix, d.numRows, myRows, myCols, myRank, numProcs);
    
    
    // Free dynami memory
    free(counterStorage);
    free(counter);

    free(bulkStorage);
    free(matrix);

    // Print runtimes to stderr so stdout can be piped to /dev/null
    endTime = MPI_Wtime();
    if (myRank == 0) {
       fprintf(stderr, "%d,%d,%d,%d,%d,%.15f,%.15f,%.15f\n", numProcs,
                     d.numRows, d.numCols, printMod, numIterations,
                     seqToPar-startTime, parToSeq-startTime, endTime-startTime); 
    }

    MPI_Finalize();
    return 0;
}




void readRowStripedMatrix(char* filename, int*** subMatrix, int** bulkStorage,
                          Dimensions* dimension, int myRank, int numProcs) {

    int** myMatrix;  // Dereferennced version of subMatrix
    int*  myStorage; // Dereferenced version of bulkStorage

    int* numRows;     // Rows and cols in global matrix
    int* numCols; 
    int myRows;       // Rows and cols in my portion of matrix
    int myCols;

    FILE* matrixFile; // File pointer for matrix file
    int bytesRead;    // Used with fread to see how much data was read

    int myLow;        // Low for current process receiving matrix data
    int numRows;         // How many rows a process has, used for distribution
    int nextLow;      // Low for next process receiving matrix data

    int i;
    int r;
    int c;
    char junk;        // Somewhere to toss newlines

    MPI_Status status;

    // Point to the varibles inside dimension
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
    myRows = BLOCK_SIZE(myRank, numProcs, *numRows);
    myCols = *numCols;

    *bulkStorage = (int*)  malloc(myRows * myCols * sizeof(int));
    *subMatrix   = (int**) malloc(myRows * sizeof(int*));

    myMatrix  = *subMatrix;
    myStorage = *bulkStorage;


    // Exit if memory allocation failed
    if (bulkStorage == NULL || subMatrix == NULL) {
        MPI_Abort(MPI_COMM_WORLD, MALLOC_ERROR);
    }


    // Link up sub-matrtix
    myMatrix[0] = *bulkStorage;
    for (i = 1; i < myRows; ++i) {
        myMatrix[i] = myMatrix[i-1] + myCols;
    }


    // Broadcast matrix data
    if (myRank == (numProcs - 1)) {

        myLow = 0;
        for (i = 0; i < numProcs; ++i) {
            // Use nextLow to avoid redundent computation
            nextLow = BLOCK_LOW(i+1, numProcs, *numRows);
            numRows = nextLow - myLow;

            // Setup myLow for next iteration
            myLow = nextLow;

            // Read in rows
            bytesRead = 0;
            for (r = 0; r < numRows; ++r) {

                // Remove newline character
                if (fscanf(matrixFile, "%c", &junk) != 1) {
                    MPI_Abort(MPI_COMM_WORLD, OPEN_FILE_ERROR);
                }
                
                // Read row and add 0's to front and back of row
                for (int c = 0; c < numCols; ++c) {
                    bytesRead += fscanf(matrixFile, "%d", myMatrix[r]+c);
                }
            }

            if (bytesRead != numRows * (*numCols)) {
                MPI_Abort(MPI_COMM_WORLD, OPEN_FILE_ERROR);
            }


            // I don't need to send data to myself
            if (i != myRank) {
                MPI_Send(myStorage, numRows * myCols, MPI_CHAR, i,
                            DATA_MSG, MPI_COMM_WORLD);
            }
        }

        close(matrixFile);

    } else {
        // Receive matrix data
        MPI_Recv(myStorage, myRows * myCols, MPI_CHAR, numProcs - 1,
                    DATA_MSG, MPI_COMM_WORLD, &status);
    }

    for (i = 0; i < myRows*myCols; ++i) {
        if ((*bulkStorage)[i] == LIVE) {
            (*bulkStorage)[i] = 1;
        } else {
            (*bulkStorage)[i] = 0;
        }
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


void printRowStripedMatrix(long long** subMatrix, int numRows, 
                            int myRows, int myCols, int myRank, int numProcs) {
    
    long long*  bulkStorage;     // Temporary storage for data from other processes
    long long** receivedMatrix;

    int i;

    int maxRows;
    int myLow;
    int nextLow;
    int size;

    MPI_Status status;
    int prompt;


    if (myRank == 0) {
        // Print my submatrix
        printSubmatrix(subMatrix, myRows, myCols);

        // Print everyone elses
        if (numProcs > 1) {

            // Allocate and storage and hookup 2D matrix
            maxRows = BLOCK_SIZE(numProcs-1, numProcs, numRows);
            bulkStorage    = (long long*)  malloc(maxRows * myCols 
                                                    * sizeof long long);
            receivedMatrix = (long long**) malloc(maxRows * sizeof(long long*));

            if (bulkStorage == NULL || receivedMatrix == NULL) {
                MPI_Abort(MPI_COMM_WORLD, MALLOC_ERROR);
            }

            receivedMatrix[0] = bulkStorage;
            for (i = 1; i < maxRows; ++i) {
                receivedMatrix[i] = receivedMatrix[i-1] + myCols;
            }

            
            // Receive matrices from everyone else and print them out
            myLow = BLOCK_LOW(1, numProcs, numRows);
            for (i = 1; i < numProcs; ++i) {
                // Calculate size of matrix to be received from process i
                nextLow = BLOCK_LOW(i+1, numProcs, numRows);
                size = nextLow - myLow + 2;
                myLow = nextLow;

                MPI_Send(&prompt, 1, MPI_INT, i, PROMPT_MSG, MPI_COMM_WORLD);


                MPI_Recv(bulkStorage, size*myCols, MPI_LONG_LONG, i, 
                            RESPONSE_MSG, MPI_COMM_WORLD, &status);

                printSubmatrix(receivedMatrix, size, myCols);
            }
    
            free(receivedMatrix);
            free(bulkStorage);
        }

    } else {
        // If I am not process 0, send my submatrix to him
        MPI_Recv(&prompt, 1, MPI_INT, 0, PROMPT_MSG, MPI_COMM_WORLD, &status);
        MPI_Send(*subMatrix, myRows*myCols, MPI_CHAR, 0, 
                    RESPONSE_MSG, MPI_COMM_WORLD);
    }
}


void printSubmatrix(long long **subMatrix, int numRows, int numCols) {
    int r;
    int c;

    for (r = 0; r < numRows; ++r) {
        for (c = 0; c < numCols; ++c) {
            printf("%d ", subMatrix[r][c]);
       }
       printf("\n");
    }
}










