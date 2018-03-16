#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ARRAY_SIZE 128

long long sum_per_process(long long* array, int array_size);
long long MPI_P2P_reduce(long long* array, int array_size, int world_size, int rank);

// MAIN ===========================================================================================
int main(int argc, char** argv) {

    MPI_Init(NULL, NULL);
    const long long ACTUAL_SUM  = (0.5*(ARRAY_SIZE*ARRAY_SIZE) - 0.5*ARRAY_SIZE);


    // Get the number of processes and rank for each process
    int world_size;
    int rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);


    if (world_size % 2 != 0) {
	fprintf(stderr, "ERROR: use a multiple of two processes\n");
	exit(EXIT_FAILURE);
    }

    long long* array = NULL;

    if (rank == 0) {
        array = (long long*) malloc(ARRAY_SIZE * sizeof(long long));
        if (array == NULL) {
            fprintf(stderr, "ERROR: malloc() failed\n");
            exit(EXIT_FAILURE);
        }

        // Initialize array values to equal their index value
        for (int i = 0; i < ARRAY_SIZE; i++) {
            array[i] = i;
        }
    }

    int elements_per_proc = ARRAY_SIZE/world_size;

    long long* sub_array = (long long*) malloc(sizeof(long long) * elements_per_proc);
    if (sub_array == NULL) {
        fprintf(stderr, "ERROR: malloc() failed\n");
        exit(EXIT_FAILURE);
    }

    MPI_Scatter(array, elements_per_proc, MPI_LONG_LONG, sub_array, elements_per_proc, MPI_LONG_LONG, 0, MPI_COMM_WORLD);

    long long homemade_sum = MPI_P2P_reduce(sub_array, elements_per_proc, world_size, rank);
	if (rank == 0)
		printf("MPI_P2P_Reduce calculated a sum of %lld\n", homemade_sum);



    long long process_sum = sum_per_process(sub_array, elements_per_proc);

    long long mpi_sum;
    MPI_Reduce(&process_sum, &mpi_sum, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
		if ((ACTUAL_SUM != mpi_sum)  ||  (ACTUAL_SUM != homemade_sum)) {
	    	printf("Sum failed...\n");
		}

    	printf(" Actual Sum: %lld\n MPI Sum: %lld\n Homemade Sum: %lld\n", ACTUAL_SUM, mpi_sum, homemade_sum);
    }




    MPI_Finalize();
    return EXIT_SUCCESS;
}



// SUM PER PROCESS ================================================================================
inline long long sum_per_process(long long* array, int array_size) {

    long long sum = 0;
    for (int i = 0; i < array_size; i++)
		sum += array[i];

    return sum;
}


// MPI P2P REDUCE ================================================================================
long long MPI_P2P_reduce(long long* array, int array_size, int world_size, int rank) {


    int total_reductions = log2(world_size);
	if (rank == 0) printf("\nTOTAL REDUCTIONS == %d\n\n", total_reductions);
    MPI_Request* receive_requests = malloc(total_reductions * sizeof(MPI_Request));


    int receivers = 2;
    int num_receives = 0;
    int source = 0;
    long long* received_sum = malloc(total_reductions * sizeof(long long));

	// Dynamic receives, different ranks receive a different number of times
    for (int i = 0;  i < total_reductions;  i++, receivers*=2) {
		if (rank == 0  ||  (rank%receivers == 0 && rank >= receivers)) {
	    	num_receives++;

	    	if (i == 0)
				source = rank+1;
	    	else
				source = rank + ((int)pow(2,i));


    		MPI_Irecv((received_sum + i), 1, MPI_LONG_LONG, source, 0, MPI_COMM_WORLD, (receive_requests + i));

			if (rank == 0 || rank == 16)
        		printf("Rank %d: receiving from %d\n", rank, source);

		} else
	    	break;
    }


    //MPI_Barrier(MPI_COMM_WORLD);
    //printf("Process %d posted %d receives\n", rank, num_receives);


    long long process_sum = sum_per_process(array, array_size);
	printf("Rank %d: initial process sum: %lld\n", rank, process_sum);

	// every process except for zero posts one send
	int destination = 0;
	int dest_found = 0;
	int modifier = 2;
	int num_sends = 0;
	int i = 0;
	MPI_Request send_request;

	if (rank > 0) {
		while (!dest_found) {
			if (rank % modifier != 0) {
				if (i == 0) destination = rank-1;
				else destination = rank - (int)pow(2,i);

				dest_found = 1;
				num_sends++;

			} else if (rank == modifier) {
				destination = 0;
				dest_found = 1;
				num_sends++;
			}

			i++;
			modifier *= 2;
		}
	}

	printf("Rank %d: waiting for %d messages\n", rank, num_receives);
	for (int j = 0; j < num_receives; j++) {
		MPI_Wait((receive_requests + j), MPI_STATUS_IGNORE);
		process_sum += received_sum[j];
		printf("Rank %d: received %lld in message %d;  new sum is %lld\n", rank, received_sum[j], j+1, process_sum);
	}

	printf("Rank %d: finished receiving\n", rank);

	if (rank > 0) {
		printf("Rank %d: sending %lld to rank %d\n", rank, process_sum, destination);
		MPI_Isend(&process_sum, 1, MPI_LONG_LONG, destination, 0, MPI_COMM_WORLD,  &send_request);
	}

	//printf("Process %d posted %d send to Process %d\n", rank, num_sends, destination);

	printf("Rank %d: Exiting\n", rank);
    return process_sum;
}
