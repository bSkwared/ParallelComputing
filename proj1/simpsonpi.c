//******************************************************************************
// simpsonpi.c
//
// Summary: This file contains code for estimating pi using Simpson's method in
// parallel. The loop uses striping to parallelize the work.
//
// Author: Blake Lasky
// Created: Sept 2016
//******************************************************************************

#include <stdio.h>
#include <mpi.h>

int main(int argc, char *argv[]) {
    
    // The actual value of pi
    const double PI = 3.141592653589793;
    
	double wholePi;   // Sum of all processes results for pieces of pi
	double pieceOfPi; // An individual processes result

    double startTime; // Seconds from epoch to the start of the loop
    double   endTime; // Seconds from epoch to the end of the combine

	int numIntervals; // How many subintervals to use

	int myRank;       // Which number process I am [0, (n-1)]
    int numProcs;     // How many processes there are going to be

    
    // Make sure user provided number of iterations
    if (argc != 2) {
        printf("\nUsage: %s numIterations\n", argv[0]);
        return 1;
    }

    // Get number of iteratinos and make sure it is positive
    numIntervals = atoi(argv[1]);
    if (numIntervals <= 0) {
        printf("\nError: number of iterations must be a positive integer\n");
        return 2;
    }

    // Begin MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

    startTime = MPI_Wtime();


    // Loop through Simpson's method
    pieceOfPi = 0.0;
    int i;
    for (i = 1 + myRank; i <= numIntervals/2; i += numProcs) {
        double x = (double) (2*i - 1) / (double) numIntervals;
        pieceOfPi += 16.0 / (1.0 + x * x);

        x = (double) 2*i / (double) numIntervals;
        pieceOfPi += 8.0 / (1.0 + x * x);
    }


    // Get whole pi from all the pieces
    MPI_Reduce(&pieceOfPi, &wholePi, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    

    // Only one process should print results
	if (myRank == 0) {
		// Finalize calculation and get end time
        wholePi += 2.0;
        wholePi /= (3.0 * numIntervals);
        endTime = MPI_Wtime();

        // Calculate the percent difference between our pi and actual pi
        double diff = PI - wholePi;
        if (diff < 0) {
            diff = -diff;
        }
        diff = diff * 2 / (PI + wholePi);

        // Print data: numProcs,numIntervals,percentDiff,runtime,ticktime
        printf("%d,%d,", numProcs, numIntervals);
        printf("%.15f,%.15f,%.15f\n", diff, endTime - startTime, MPI_Wtick());
	}

    MPI_Finalize();

    return 0;
}
