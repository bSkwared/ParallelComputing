#include <stdio.h>
#include <math.h>

int main(int argc, char *argv[]) {

	int procID;       // Which number process I am [0, (n-1)]
    int numProcs;     // How many processes there are going to be

    // Make sure user pricded correct number of arguments
    if (argc != 3) {
        printf("\nUsage: %s numProcs procID\n", argv[0]);
        return 1;
    }

    // Get number of procs
    numProcs = atoi(argv[1]);
    if (numProcs == 0) {
        printf("\nError: number of processes must be a positive integer\n");
        return 2;
    }

    // Get process ID
    procID = atoi(argv[2]);
    if (procID < 0 || numProcs <= procID) {
        printf("\nError: procID must be less than the number of procs\n");
        return 3;
    }

    int lgn = floor(log2(numProcs));
    int curDiff = pow(2, lgn);

    // Only some processes will communicate on the first step
    // After this step, the number of procs will be a power of two
    if (procID + curDiff >= numProcs && procID < curDiff) {
        curDiff /= 2;
    }

    // Receive messages from higher procs
    while (procID < curDiff && curDiff > 0) {
        printf("Message received from task %d\n", procID + curDiff);
        curDiff /= 2;
    }

    // Send messages to lower procs
    if (procID != 0) {
        printf("Message sent to task %d\n", procID - curDiff);
    }
    
	return 0;
}
