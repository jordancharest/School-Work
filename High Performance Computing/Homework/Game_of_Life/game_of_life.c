/***************************************************************************/
/* Game of Life Simulator       ********************************************/
/* Written by Jordan Charest    ********************************************/
/***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

#include <mpi.h>
#include "clcg4.h"

#define ALIVE 1
#define DEAD  0


// Global ========================================================================================
const unsigned int BOARD_SIZE = 1024;
const unsigned int TICKS = 128;
const double THRESHOLD = 0.25;
const int MAX_WORLD_SIZE = 128;

unsigned int rows_per_thread = 0;
unsigned int rows_per_rank = 0;
unsigned int num_threads = 0;
int rank = 0;
int world_size = 0;

pthread_t master_thread = 0;
pthread_barrier_t barrier;
pthread_mutex_t rank_lock = PTHREAD_MUTEX_INITIALIZER;
int thread_organizer = 1;


// Function Declarations =========================================================================
void sim_init();
void validate();

int** matrix_alloc(unsigned int rows, unsigned int cols);
void matrix_free( int **matrix, unsigned int rows);
void first_generation(int** sub_matrix);

void* simulation(void* args);
void new_generation(int** sub_matrix, int thread_rank, int** neighbors);
void add_ghost_data(int** sub_matrix, int* top, int* bottom);
int count_neighbors(int** sub_matrix, int row, int col);

int** pooling(int** sub_matrix);

void print_board(int** board, unsigned int rows, unsigned int cols, char spacer);
int modulo (int numerator, int denominator);

// MAIN ==========================================================================================
int main(int argc, char **argv) {

    MPI_Init( &argc, &argv);

	// Initialize run parameters and ensure they are valid
	sim_init();

	// Init 16,384 RNG streams - each rank has an independent stream
    InitDefault();

	// Initialize pthread barrier to block all threads in a process
	pthread_barrier_init(&barrier, NULL, num_threads);

#if 1
	if (rank == 0) {
		printf("Run parameters:\n");
		printf(" Board Size: %d\n", BOARD_SIZE);
		printf(" MPI World Size: %d\n", world_size);
		printf(" Rows per rank: %u\n", rows_per_rank);
		printf(" Threads per rank: %d\n", num_threads);
		printf(" Rows per thread: %u\n\n\n", rows_per_thread);
		fflush(stdout);
	}
#endif

	// Compute Operations ---------------------------------------------------------------

	// Time stamp for compute time
    MPI_Barrier( MPI_COMM_WORLD );
	double start_time = 0;
	if (rank == 0)
		start_time = MPI_Wtime();

	// allocate and initialize the world; add two more rows for ghost data
	int** sub_matrix = matrix_alloc(rows_per_rank+2, BOARD_SIZE);
	first_generation(sub_matrix);

	// enter the simulation with all child pthreads
	pthread_t* tid = malloc(sizeof(pthread_t) * num_threads);
	for (int thread = 1; thread < num_threads; thread++) {
		if ( pthread_create( &tid[thread], NULL, simulation, (void*)sub_matrix ) != 0 ) {
			fprintf(stderr, "ERROR: Could not create thread\n");
			exit(EXIT_FAILURE);
		}
	}

	// enter the simulation with the main thread
	simulation((void*)sub_matrix);
	pthread_barrier_wait(&barrier);

	if (rank == 0)
		printf("\n\nTotal Compute Time: %lf\n", (MPI_Wtime() - start_time));


	// I/O Operations ----------------------------------------------------------------

	// Time stamp for I/O Time
	MPI_Barrier(MPI_COMM_WORLD);
	start_time = 0;
	if (rank == 0)
		start_time = MPI_Wtime();

    // open and write to file
    MPI_File fh;
    MPI_Status write_status;
    int offset = 0;
    MPI_File_open( MPI_COMM_WORLD, "final_world_state.txt", MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &fh );

    for (int i = 1; i <= rows_per_rank; i++) {
        offset = (rank * rows_per_rank * BOARD_SIZE) + ((i-1) * BOARD_SIZE);
        MPI_File_write_at( fh, offset, sub_matrix[i], BOARD_SIZE, MPI_UINT8_T, &write_status);
    }


	if (rank == 0)
		printf("Total I/O Time: %lf\n", (MPI_Wtime() - start_time));


	// Cleanup
	matrix_free(sub_matrix, rows_per_rank+2);

    MPI_Barrier( MPI_COMM_WORLD );
    MPI_Finalize();
    return EXIT_SUCCESS;
}


// SIM INIT ======================================================================================
void sim_init() {
    MPI_Comm_size( MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank( MPI_COMM_WORLD, &rank);

	// define run parameters
	master_thread = pthread_self();	// each MPI rank will have a master thread that will handle all communication
	num_threads = MAX_WORLD_SIZE/world_size;
	rows_per_rank = BOARD_SIZE/world_size;	// DON'T GET CONFUSED WITH rows_per_thread! (they are equal if num_threads == 0)
	rows_per_thread = rows_per_rank/num_threads;

	// ensure they are valid
	validate();

}

// VALIDATE MPI ==================================================================================
/* Ensure that the simulation run parameters are valid											*/
void validate() {

	// ensure that the world size is a power of 2
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

	// ensure the world size is not more than allowed on the system
	if (world_size > MAX_WORLD_SIZE) {
		fprintf(stderr, "ERROR: Adjust max world size to continue. Kratos: 128; Blue Gene/Q: 256");
		exit(EXIT_FAILURE);
	}

	// ensure that there aren't too many threads for the given board (# threads <= # rows)
	if (rows_per_thread == 0) {
		fprintf(stderr, "Rows per thread = 0! Your world size is too large for the given board size\n");
		exit(EXIT_FAILURE);
	}

}

// FIRST GENERATION ==============================================================================
/* Randomly generate the first generation; Don't populate the first and last rows since
	they are ghost rows																			*/
void first_generation(int** sub_matrix) {
	for (int row = 1; row < rows_per_rank+1; row++)		// leave top and bottom rows uninitialized
		for (int col = 0; col < BOARD_SIZE; col++)
			if (GenVal(rank*rows_per_rank + row) < THRESHOLD)
				sub_matrix[row][col] = ALIVE;
}


// CONTROL THE GAME OF LIFE SIMULATION ===========================================================
void* simulation(void* args) {
	int** sub_matrix = (int**)args;

	int* top_row = calloc(BOARD_SIZE, sizeof(*top_row));
	int* bottom_row = calloc(BOARD_SIZE, sizeof(*bottom_row));
	MPI_Request top_request;
	MPI_Request bottom_request;
	MPI_Request send_request;

	// Modulo operator in C doesn't work on negative numbers the way I need it to
	int top_source = modulo(rank-1, world_size);
	int bottom_source = modulo(rank+1, world_size);

	int top_destination = modulo(rank-1, world_size);
	int bottom_destination = modulo(rank+1, world_size);

	int thread_rank = 0;

	int** neighbors = matrix_alloc(rows_per_rank+2, BOARD_SIZE);

	// arbitrary ranking system to determine which rows each thread owns; master thread is 0
	if (pthread_self() != master_thread) {
		pthread_mutex_lock(&rank_lock);
			thread_rank = thread_organizer;
			thread_organizer++;
		pthread_mutex_unlock(&rank_lock);
	}


	for (int gen = 0; gen < TICKS; gen++) {
		if (master_thread == pthread_self()) {
			#if 0
				printf("RANK %d: Receiving top row from rank %d and bottom row from rank %d\n", rank, top_source, bottom_source);
			#endif


			MPI_Irecv(top_row, BOARD_SIZE, MPI_INT, top_source, 100, MPI_COMM_WORLD, &top_request);
			MPI_Irecv(bottom_row, BOARD_SIZE, MPI_INT, bottom_source, 200, MPI_COMM_WORLD, &bottom_request);
		}


		// CRITICAL vvvvvvvv Master thread receiving ghost data and incorporating into the sub matrix
		// all threads must be done with the previous generation before the master thread attempts to update ghost data
		pthread_barrier_wait(&barrier);

		if (master_thread == pthread_self()) {
			MPI_Isend(sub_matrix[1], BOARD_SIZE, MPI_INT, top_destination, 200, MPI_COMM_WORLD, &send_request);
			MPI_Isend(sub_matrix[rows_per_rank], BOARD_SIZE, MPI_INT, bottom_destination, 100, MPI_COMM_WORLD, &send_request);

			// wait to receive both top and bottom ghost rows
			MPI_Wait(&top_request, MPI_STATUS_IGNORE);
			MPI_Wait(&bottom_request, MPI_STATUS_IGNORE);

			add_ghost_data(sub_matrix, top_row, bottom_row);

		}	// end master thread incorporation of ghost data

		pthread_barrier_wait(&barrier);

		// CRITICAL  ^^^^^^^^ Master thread finished receiving ghost data and incorporating into the sub matrix
		// no thread can attempt to count neighbors until the master thread has incorporated the ghost data


        // count how many neighbors each cell has
		for (int row = thread_rank*rows_per_thread+1;  row < (thread_rank*rows_per_thread + rows_per_thread + 1);  row++)
			for (int col = 0; col < BOARD_SIZE; col++)
				neighbors[row][col] = count_neighbors(sub_matrix, row, col);

		// update to the next generation, but only if everyone is done counting their neighbors
		pthread_barrier_wait(&barrier);
		new_generation(sub_matrix, thread_rank, neighbors);

	} // end simulation

	if (pthread_self() != master_thread) {
		pthread_detach(pthread_self());
		pthread_barrier_wait(&barrier);
		return EXIT_SUCCESS;
	}

    matrix_free(neighbors, rows_per_rank+2);
	return 0;
}

// NEW GENERATION ================================================================================
void new_generation(int** sub_matrix, int thread_rank, int** neighbors) {

	for (int row = thread_rank*rows_per_thread+1;  row < (thread_rank*rows_per_thread + rows_per_thread + 1);  row++) {
		for (int col = 0; col < BOARD_SIZE; col++) {

			// Sometimes we will ignore a cell's neighbors and just randomly choose if it is alive or dead
			if (GenVal(rank*rows_per_rank + row) < THRESHOLD) {
				if (GenVal(rank*rows_per_rank + row) < 0.5)
					sub_matrix[row][col] = ALIVE;
				else
					sub_matrix[row][col] = DEAD;


			// the rest of the time we will determine a cell's life by its neighbors
			} else {

				// alive cells with fewer than 2 neighbors will die
				if (sub_matrix[row][col]  &&  neighbors[row][col] < 2)
					sub_matrix[row][col] = DEAD;

				// alive cells with more than 3 neighbors will die
				else if (sub_matrix[row][col]  &&  neighbors[row][col] > 3)
					sub_matrix[row][col] = DEAD;

				// dead cells with exactly 3 neighbors will be born
				else if (!sub_matrix[row][col]  &&  neighbors[row][col] == 3)
					sub_matrix[row][col] = ALIVE;
			}
		}
	}
}

// COUNT NEIGHBORS ===============================================================================
inline int count_neighbors(int** sub_matrix, int row, int col) {

	int total = 0;
	int y, x;

	for (int i = -1; i < 2; i++) {
		for (int j = -1; j < 2; j++) {
			// x may be out of bounds; y will not be due to ghost rows on top/bottom
			y = i + row;
			x = modulo(col+j, BOARD_SIZE);

			if (sub_matrix[y][x] && !(y == row  && x == col))
				total++;

		}
	}

	return total;
}


// ADD GHOST DATA ================================================================================
void add_ghost_data(int** sub_matrix, int* top, int* bottom) {
	for (int i = 0; i < BOARD_SIZE; i++) {
		sub_matrix[0][i] = top[i];
		sub_matrix[rows_per_rank+1][i] = bottom[i];
	}
}


// MATRIX ALLOCATION =============================================================================
/* allocate an array of pointers to ints, then allocate a row/array of ints and assign each
    int pointer that row (ROW MAJOR)                                                            */
int** matrix_alloc(unsigned int rows, unsigned int cols) {
    int** array = calloc(rows, sizeof(*array));
    if ( array == NULL ) {
        fprintf( stderr, "ERROR: calloc() failed\n" );
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < rows; i++){
        array[i] = calloc(cols, sizeof(*array[i]));
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


// PRINT BOARD ===================================================================================
void print_board(int** board, unsigned int rows, unsigned int cols, char spacer) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%d%c", board[i][j], spacer);
        }
        printf("\n");
    }
}


// MODULO ========================================================================================
inline int modulo (int numerator, int denominator) {
	return ((numerator % denominator + denominator) % denominator);
}
