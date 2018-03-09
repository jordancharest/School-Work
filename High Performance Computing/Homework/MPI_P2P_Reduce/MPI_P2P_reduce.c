#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE 100000




// MAIN ============================================================================================
int main(int argc, char** argv) {

    MPI_Init(NULL, NULL);

    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);


    if (rank == 0) {
        ARRAY_T* array = malloc(ARRAY_SIZE * sizeof(ARRAY_T));
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
    int* sub_array = malloc(sizeof(MPI_LONG_LONG) * elements_per_proc);
    if (sub_array == NULL) {
        fprintf(stderr, "ERROR: malloc() failed\n");
        exit(EXIT_FAILURE);
    }

    MPI_Scatter(array, elements_per_proc, MPI_LONG_LONG, sub_array, elements_per_proc, MPI_LONG_LONG, 0, MPI_COMM_WORLD);

    return EXIT_SUCCESS;
}
