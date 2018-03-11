#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE 100

long long sum_per_process(long long* array, int array_size);


// MAIN ============================================================================================
int main(int argc, char** argv) {

    MPI_Init(NULL, NULL);

    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
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

    long long process_sum = sum_per_process(sub_array, elements_per_proc);
    long long total_sum;
    MPI_Reduce(&process_sum, &total_sum, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Sum: %lld\n", total_sum);
    }




    MPI_Finalize();
    return EXIT_SUCCESS;
}


// SUM PER PROCESS =================================================================================
inline long long sum_per_process(long long* array, int array_size) {

    long long sum = 0;
    for (int i = 0; i < array_size; i++) {
	sum += array[i];
    }

    return sum;
}
