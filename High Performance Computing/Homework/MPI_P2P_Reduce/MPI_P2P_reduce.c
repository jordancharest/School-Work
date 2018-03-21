#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define BGQ 1

#ifdef BGQ
#include<hwi/include/bqc/A2_inlines.h>
#else
#define GetTimeBase MPI_Wtime
#endif // BGQ

long long sum_per_process(long long* array, int array_size);
long long MPI_P2P_reduce(long long* array, int array_size, int world_size, int rank);

// MAIN ===========================================================================================
int main(int argc, char** argv) {

    MPI_Init(NULL, NULL);
    const long long ARRAY_SIZE = 1073741824;
    const long long ACTUAL_SUM  = (0.5*(ARRAY_SIZE*ARRAY_SIZE) - 0.5*ARRAY_SIZE);

    double time_in_secs = 0;
    double processor_frequency = 1600000000.0;
    unsigned long long start_cycles = 0;
    unsigned long long end_cycles = 0;


    // Get the number of processes and rank for each process
    int world_size;
    int rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	// ensure the world size is a power of 2
	int i = 1;
	for (; i < 20; i++) {
		if (world_size == pow(2,i))
			break;
	}

    if (i == 20) {
		if (rank == 0)
			fprintf(stderr, "ERROR: use a multiple of two processes\n");
		exit(EXIT_FAILURE);
    }

    long long* array = NULL;
    int elements_per_proc = ARRAY_SIZE/world_size;

    long long* sub_array = malloc(sizeof(long long) * elements_per_proc);
    if (sub_array == NULL) {
        fprintf(stderr, "ERROR: malloc() failed\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0, j = rank*elements_per_proc; j < rank*elements_per_proc + elements_per_proc; i++, j++) {
        sub_array[i] = j;
    }

    // My implementation of MPI_Reduce (MPI_P2P_Reduce)
	start_cycles = GetTimeBase();
    long long homemade_sum = MPI_P2P_reduce(sub_array, elements_per_proc, world_size, rank);
	end_cycles = GetTimeBase();
	time_in_secs = ((double)(end_cycles - start_cycles))/processor_frequency;

	if (rank == 0)
		printf("MPI_P2P_Reduce time: %f\n", time_in_secs);


    // My implementation of MPI_Reduce (MPI_P2P_Reduce)
	start_cycles = GetTimeBase();
    long long process_sum = sum_per_process(sub_array, elements_per_proc);
    long long mpi_sum;
    MPI_Reduce(&process_sum, &mpi_sum, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
	end_cycles = GetTimeBase();
	time_in_secs = ((double)(end_cycles - start_cycles))/processor_frequency;

	if (rank == 0)
		printf("MPI_Reduce time: %f\n", time_in_secs);

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

			#ifdef DEBUG_MODE
				if (rank == 0 || rank == 16)
    	    		printf("Rank %d: receiving from %d\n", rank, source);
			#endif

		} else
	    	break;
    }


    long long process_sum = sum_per_process(array, array_size);

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


	#ifdef DEBUG_MODE
		printf("Rank %d: waiting for %d messages\n", rank, num_receives);
	#endif

	for (int j = 0; j < num_receives; j++) {
		MPI_Wait((receive_requests + j), MPI_STATUS_IGNORE);
		process_sum += received_sum[j];

		#ifdef DEBUG_MODE
			printf("Rank %d: received %lld in message %d;  new sum is %lld\n", rank, received_sum[j], j+1, process_sum);
		#endif
	}


	if (rank > 0) {
		#ifdef DEBUG_MODE
			printf("Rank %d: sending %lld to rank %d\n", rank, process_sum, destination);
		#endif

		MPI_Isend(&process_sum, 1, MPI_LONG_LONG, destination, 0, MPI_COMM_WORLD,  &send_request);
	}


	#ifdef DEBUG_MODE
		printf("Rank %d: Exiting\n", rank);
    #endif


	return process_sum;
}
