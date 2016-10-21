#include <stdio.h>
#include <mpi.h>

#define n 100000

double f(int i) {
    double x;
    x = (double) i / (double) n;
    return 4.0 / (1.0 + x * x);    
}

int main(int argc, char *argv[]) {
    double area;
    double final;
    int i;

    int myRank, numProcs;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);


    for (i = 1 + myRank; i <= n/2; i += numProcs) {
        area += 4.0 * f(2 * i - 1) + 2 * f(2*i);
    }


    printf("%d: made it here\n", myRank);
    system("hostname");

    MPI_Reduce(&area, &final, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    
    printf("%d: passed reduce\n", myRank);
    system("hostname");


    if (myRank == 0) {
        final += f(0) - f(n);
        final /= (3.0 * n);
        printf("Approximation of pi: %.17f\n", final);
    }

    MPI_Finalize();

    return 0;
}
