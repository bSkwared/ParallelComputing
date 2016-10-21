#include <mpi.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
	int numProcs, myRank;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
	MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

	//if (myRank == 0) {
		printf("Hello World! : %d\n", myRank);
		system("hostname");
	//}

	MPI_Finalize();
	return 0;
}
