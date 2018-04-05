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
const unsigned int BOARD_SIZE = 16;
const unsigned int TICKS = 1;
const double THRESHOLD = 0.25;
const unsigned int ROWS_PER_THREAD = 64;

unsigned int rows_per_rank = 0;
unsigned int num_threads = 0;
int rank = 0;
int world_size = 0;

pthread_t master_thread = 0;
pthread_barrier_t barrier;
pthread_mutex_t rank_lock = PTHREAD_MUTEX_INITIALIZER;
int thread_organizer = 1;


// Function Declarations =========================================================================
void validate_MPI();
int** matrix_alloc(unsigned int rows, unsigned int cols);
void matrix_free( int **matrix, unsigned int rows);
void first_generation(int** sub_matrix);

void* simulation(void* args);
void new_generation(int** sub_matrix, int thread_rank);
void add_ghost_data(int** sub_matrix, int* top, int* bottom);
int count_neighbors(int** sub_matrix, int row, int col);

void print_board(int** board, unsigned int rows, unsigned int cols);
int modulo (int numerator, int denominator);

// MAIN ==========================================================================================
int main(int argc, char **argv) {

    MPI_Init( &argc, &argv);
    MPI_Comm_size( MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank( MPI_COMM_WORLD, &rank);
	validate_MPI();
	master_thread = pthread_self();	// each MPI rank will have a mster thread that will handle all communication

	num_threads = 128/world_size;
	rows_per_rank = BOARD_SIZE/world_size;	// DON'T GET CONFUSED WITH rows_per_thread! (they are equal if num_pthreads == 0)

	// Init 16,384 RNG streams - each rank has an independent stream
    InitDefault();

	// Initialize pthread barrier to block all threads in a process
	pthread_barrier_init(&barrier, NULL, num_threads);

#ifdef DEBUG
	if (rank == 0) {
		printf("Run parameters:\n");
		printf(" Board Size: %d\n", BOARD_SIZE);
		printf(" MPI World Size: %d\n", world_size);
		printf(" Threads per rank: %d\n", num_threads);
		printf(" Rows per rank: %u\n", rows_per_rank);
		printf(" Rows per thread: %u\n\n\n", ROWS_PER_THREAD);
	}
#endif

    //printf(" Rank %d of %d has been started with a first Random Value of %lf\n",
	//   rank, world_size, GenVal(rank));

    MPI_Barrier( MPI_COMM_WORLD );
	double start_time = 0;
	if (rank == 0)
		start_time = MPI_Wtime();

	// allocate and initialize the world; add two more rows for ghost data
	int** sub_matrix = matrix_alloc(rows_per_rank+2, BOARD_SIZE);
	first_generation(sub_matrix);


#ifdef DEBUG
	int message = 1;
	if (rank == 1)
		MPI_Recv(&message, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);


	print_board(sub_matrix, rows_per_rank+2, BOARD_SIZE);
	printf("Creating %u child threads\n", num_threads-1);


	if (rank == 0)
		MPI_Send(&message, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
#endif

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


	// PASS BACK CHILD THREAD ID? POSSIBLY DETACH CHILD THREADS?
	// Join child threads
/*	unsigned int* x;
	for (int i = 0; i < num_threads; i++) {
		if ( pthread_join(tid[i], (void**)&x) != 0 )
			fprintf( stderr, "MAIN: Could not join child thread\n");
	}
*/



//	matrix_free(sub_matrix, rows_per_rank);



//	if (rank == 0)
//		printf("Total time: %lf\n", (MPI_Wtime() - start_time));

    MPI_Barrier( MPI_COMM_WORLD );
    MPI_Finalize();
    return EXIT_SUCCESS;
}



// VALIDATE MPI ==================================================================================
/* Ensure that the world size is a power of 2													*/
void validate_MPI() {
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

	// arbitrary ranking system to determine which rows each thread owns; master thread is 0
	if (pthread_self() != master_thread) {
		pthread_mutex_lock(&rank_lock);
			thread_rank = thread_organizer;
			thread_organizer++;
		pthread_mutex_unlock(&rank_lock);
	}


	for (int gen = 0; gen < TICKS; gen++) {
		if (master_thread == pthread_self()) {
			#ifdef DEBUG
				printf("RANK %d: Receiving top row from rank %d and bottom row from rank %d\n", rank, top_source, bottom_source);
			#endif


			MPI_Irecv(top_row, BOARD_SIZE, MPI_INT, top_source, 100, MPI_COMM_WORLD, &top_request);
			MPI_Irecv(bottom_row, BOARD_SIZE, MPI_INT, bottom_source, 200, MPI_COMM_WORLD, &bottom_request);
		}


		// CRITICAL vvvvvvvv Master thread receiving ghost data and incorporating into the sub matrix
		// all threads must be done with the previous generation before the master thread attempts to update ghost data
		pthread_barrier_wait(&barrier);

		if (master_thread == pthread_self()) {
			#ifdef DEBUG
				printf("RANK %d: Sending top row to rank %d and bottom row to rank %d\n", rank, top_destination, bottom_destination);
			#endif


			MPI_Isend(sub_matrix[1], BOARD_SIZE, MPI_INT, top_destination, 200, MPI_COMM_WORLD, &send_request);
			MPI_Isend(sub_matrix[rows_per_rank], BOARD_SIZE, MPI_INT, bottom_destination, 100, MPI_COMM_WORLD, &send_request);

			// wait to receive both top and bottom ghost rows
			MPI_Wait(&top_request, MPI_STATUS_IGNORE);
			MPI_Wait(&bottom_request, MPI_STATUS_IGNORE);

			add_ghost_data(sub_matrix, top_row, bottom_row);
		}

		pthread_barrier_wait(&barrier);

		// CRITICAL  ^^^^^^^^ Master thread finished receiving ghost data and incorporating into the sub matrix
		// no thread can attempt to update to this generation until the master thread has incorporated the ghost data

		pthread_mutex_lock(&rank_lock);
		new_generation(sub_matrix, thread_rank);
		pthread_mutex_unlock(&rank_lock);



	#ifdef DEBUG
		if (master_thread == pthread_self()) {
			int message = 1;
			if (rank == 1)
				MPI_Recv(&message, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);


			printf("\nRANK %d:\n", rank);
			print_board(sub_matrix, rows_per_rank+2, BOARD_SIZE);


			if (rank == 0)
				MPI_Send(&message, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
		}
	#endif




	}


	return 0;
}

// NEW GENERATION ================================================================================
void new_generation(int** sub_matrix, int thread_rank) {
	int neighbors = 0;

	for (int row = 1; row < rows_per_rank+1; row++) {
		for (int col = 0; col < BOARD_SIZE; col++) {

			neighbors = count_neighbors(sub_matrix, row, col);

			// rules for population generation
			if (neighbors == 0) {
				if (GenVal(rank*rows_per_rank + row) < THRESHOLD)
					sub_matrix[row][col] = ALIVE;
			}
		}
	}
}

// COUNT NEIGHBORS ===============================================================================
inline int count_neighbors(int** sub_matrix, int row, int col) {

	int total = 0;
	int global_y;
	int global_x;

	for (int y = -1; y < 2; y++) {
/*		if (y > 1)
			fprintf(stderr, "WHAT THE FUCK - outer loop\n");
*/

		for (int x = -1; x < 2; x++) {
			if (y > 1)
				fprintf(stderr, "WHAT THE FUCK - inner loop\n");


			// x may be out of bounds; y will not be due to ghost rows on top/bottom
			global_y = y + row;
			global_x = modulo(col+x, BOARD_SIZE);

			// check if outside array bounds
			if (global_y < 0 || global_y > (rows_per_rank+1) || global_x < 0 || global_x >= BOARD_SIZE) {
				fprintf(stderr, "Row: %d, y: %d\n", row, y-row);
				fprintf(stderr, "[%d][%d]\n", y, x);
			}



			if (y > 1)
				fprintf(stderr, "WHAT THE FUCK - before sub matrix\n");


			if (sub_matrix[global_y][global_x] && !(global_y == row  && global_x == col))
				total++;


			if (y > 1)
				fprintf(stderr, "WHAT THE FUCK - after sub matrix\n");





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


// PRINT BOARD ===================================================================================
void print_board(int** board, unsigned int rows, unsigned int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%d", board[i][j]);
        }
        printf("\n");
    }
}


// MODULO ========================================================================================
inline int modulo (int numerator, int denominator) {
	return ((numerator % denominator + denominator) % denominator);
}
