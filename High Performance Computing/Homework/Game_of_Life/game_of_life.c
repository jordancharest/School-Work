/***************************************************************************/
/* Game of Life Simulator       ********************************************/
/* Written by Jordan Charest    ********************************************/
/***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>

#include <mpi.h>
#include "clcg4.h"

#define ALIVE 1
#define DEAD  0

// Global ========================================================================================
const unsigned int BOARD_SIZE = 8192;
const unsigned int TICKS = 128;
const double THRESHOLD = 0.25;

const unsigned int rows_per_thread = 64;
unsigned int rows_per_rank = 0;
unsigned int num_threads = 0;

pthread_t master_thread = 0;

// Function Declarations =========================================================================
void validate_MPI(int world_size, int rank);
int** matrix_alloc(unsigned int rows, unsigned int cols);
void matrix_free( int **matrix, unsigned int rows);
void first_generation(int** sub_matrix, int rank);

void* simulation(void* args);

// MAIN ==========================================================================================
int main(int argc, char **argv) {
    int rank;
    int world_size;

    MPI_Init( &argc, &argv);
    MPI_Comm_size( MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank( MPI_COMM_WORLD, &rank);
	validate_MPI(world_size, rank);
	master_thread = pthread_self();	// each MPI rank will have a mster thread that will handle all communication

	num_threads = 128/world_size;
	rows_per_rank = BOARD_SIZE/world_size;	// DON'T GET CONFUSED WITH rows_per_thread! (they are equal if num_pthreads == 0)

	// Init 16,384 RNG streams - each rank has an independent stream
    InitDefault();

#ifdef DEBUG
	if (rank == 0) {
		printf("Run parameters:\n");
		printf(" Board Size: %d\n", BOARD_SIZE);
		printf(" MPI World Size: %d\n", world_size);
		printf(" Threads per rank: %d\n", num_threads);
		printf(" Rows per rank: %u\n", rows_per_rank);
		printf(" Rows per thread: %u\n\n\n", rows_per_thread);
	}
#endif

    //printf(" Rank %d of %d has been started with a first Random Value of %lf\n",
	//   rank, world_size, GenVal(rank));

    MPI_Barrier( MPI_COMM_WORLD );
	double start_time = 0;
	if (rank == 0)
		start_time = MPI_Wtime();

	// allocate and initialize the world; add two more rows and columns for ghost data
	int** sub_matrix = matrix_alloc(rows_per_rank+2, BOARD_SIZE+2);
	first_generation(sub_matrix, rank);


#ifdef DEBUG
	if (rank == 0) {
		printf("Creating %u child threads\n", num_threads-1);
	}
#endif

	// enter the simulation with all child pthreads
	pthread_t* tid = malloc(sizeof(pthread_t) * num_threads);
	for (int thread = 1; thread < num_threads; thread++) {
		if ( pthread_create( &tid[thread], NULL, simulation, (void*)&sub_matrix ) != 0 ) {
			fprintf(stderr, "ERROR: Could not create thread\n");
			exit(EXIT_FAILURE);
		}
	}

	// enter the simulation with the main thread
	simulation((void*)&sub_matrix);


	matrix_free(sub_matrix, rows_per_rank);

	// Join child threads?

	if (rank == 0)
		printf("Total time: %lf\n", (MPI_Wtime() - start_time));

    MPI_Barrier( MPI_COMM_WORLD );
    MPI_Finalize();
    return EXIT_SUCCESS;
}



// VALIDATE MPI ==================================================================================
/* Ensure that the world size is a power of 2													*/
void validate_MPI(int world_size, int rank) {
	int i = 1;
	for (; i < 20; i++) {
		if (world_size == pow(2,i))
			break;
	}

	if (i == 20) {
		if (rank == 0)
			fprintf(stderr, "ERROR: Use a multiple of two MPI ranks\n");
		exit(EXIT_FAILURE);
	}
}


// MATRIX ALLOCATION =============================================================================
/* allocate an array of pointers to ints, then allocate a row/array of ints and assign each
    int pointer that row (ROW MAJOR)                                                            */
int** matrix_alloc(unsigned int rows, unsigned int cols) {
    int** array = (int **)calloc(rows, sizeof(int*));
    if ( array == NULL ){
        fprintf( stderr, "ERROR: calloc() failed\n" );
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < rows; i++){
        array[i] = (int*)calloc(cols, sizeof(int));
        if ( array[i] == NULL ){
            fprintf( stderr, "ERROR: calloc() failed\n" );
            exit(EXIT_FAILURE);
        }
    }

    return array;
}


// MATRIX FREE ===================================================================================
void matrix_free( int **matrix, unsigned int rows) {
    // free each pointer in the array
    for (int i = 0; i < rows; i++){
        free(matrix[i]);
    }

    // then free the array of pointers
    free(matrix);
    matrix = NULL;
}


// FIRST GENERATION ==============================================================================
/* Randomly generate the first generation; Don't populate the first and last rows since
	they are ghost rows																			*/
void first_generation(int** sub_matrix, int rank) {
	for (int row = 1; row != rows_per_rank; row++)
		for (int col = 1; col != BOARD_SIZE; col++)
			if (GenVal(rank*rows_per_rank + row) < THRESHOLD)
				sub_matrix[row][col] = ALIVE;
}


// UPDATE SIMULATION TO THE NEXT GENERATION ======================================================
void* simulation(void* args) {

	for (int gen = 0; gen < TICKS; gen++) {










	}


	return 0;
}
