#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>

typedef struct board_position {
    unsigned int x;
    unsigned int y;
} position_t;

typedef struct thread_args {
    int** board;
    position_t current_move;
    unsigned int visited;
} TA_t;

unsigned int max_squares = 0;
unsigned int knight_visits = 0;
int*** dead_end_boards;
unsigned int size_DEB = 8;
unsigned int index_DEB = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


unsigned int available_spaces = 0;
unsigned int rows = 0;
unsigned int cols = 0;
pthread_t masters_children[8];
pthread_t master_thread = 0;


// PRINT BOARD ===================================================================================
void print_board(int** board) {
    for (int i = 0; i < rows; i++) {
        printf("THREAD %u:", (unsigned int)pthread_self());
        if (i == 0)
            printf(" > ");
        else
            printf("   ");

        for (int j = 0; j < cols; j++) {
            if (board[i][j]) printf("k");
            else printf(".");
        }
        printf("\n");
    }
}

// MATRIX ALLOCATION =============================================================================
/* allocate an array of pointers to ints, then allocate a row/array of ints and assign each
    int pointer that row (ROW MAJOR)                                                            */
int** matrix_alloc() {
    int** array = (int **)calloc(rows, sizeof(int*));
    if ( array == NULL ){
        fprintf( stderr, "ERROR: malloc() failed\n" );
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < rows; i++){
        array[i] = (int*)calloc(cols, sizeof(int));
        if ( array[i] == NULL ){
            fprintf( stderr, "ERROR: malloc() failed\n" );
            exit(EXIT_FAILURE);
        }
    }

    return array;
}

// MATRIX FREE =================================================================================
void matrix_free( int **matrix){
    // free each pointer in the array
    for (int i = 0; i < rows; i++){
        free(matrix[i]);
    }

    // then free the array of pointers
    free(matrix);
    matrix = NULL;
}


// DEAD END ======================================================================================
void dead_end(int** board, int visited) {
    printf("THREAD %u: Dead end after move #%d\n", (unsigned int)pthread_self(), visited);


    // only add dead end boards if they are over the user-defined threshold for squares visited
    if (visited >= knight_visits) {
        /* ---------- CRITICAL ---------- */
        pthread_mutex_lock(&mutex);

            if (index_DEB == size_DEB) {
                size_DEB *= 2;
                dead_end_boards = (int***)realloc( dead_end_boards, size_DEB * sizeof(int**) );
            }

            dead_end_boards[index_DEB] = board;
            index_DEB++;

            if (visited > max_squares)
                max_squares = visited;

        pthread_mutex_unlock(&mutex);
        /* ---------- CRITICAL ---------- */
    }

    unsigned int * y = malloc( sizeof( unsigned int ) );
    *y = pthread_self();
    pthread_exit( y );
}


// NUMBER OF POSSIBLE MOVES ======================================================================
int num_possible_moves(int** board, position_t* available_moves, position_t knight, int visited) {
    unsigned int moves = 0;
    int x = knight.x;
    int y = knight.y;
    position_t space = {0,0};

    // Check 8 valid board locations                            2 spaces | 1 space
    if (x+2<cols  &&  y+1<rows  &&  board[y+1][x+2] == 0) {  //   right -> up
        space.x = x+2;
        space.y = y+1;
        available_moves[moves] = space;
        moves++;
    }
    if (x+2<cols  &&  y-1>=0    &&  board[y-1][x+2] == 0) {  //   right -> down
        space.x = x+2;
        space.y = y-1;
        available_moves[moves] = space;
        moves++;
    }
    if (x-2>=0    &&  y+1<rows  &&  board[y+1][x-2] == 0) {  //   left  -> up
        space.x = x-2;
        space.y = y+1;
        available_moves[moves] = space;
        moves++;
    }
    if (x-2>=0    &&  y-1>=0    &&  board[y-1][x-2] == 0) {  //   left  -> down
        space.x = x-2;
        space.y = y-1;
        available_moves[moves] = space;
        moves++;
    }
    if (x+1<cols  &&  y+2<rows  &&  board[y+2][x+1] == 0) {  //   up    -> right
        space.x = x+1;
        space.y = y+2;
        available_moves[moves] = space;
        moves++;
    }
    if (x-1>=0    &&  y+2<rows  &&  board[y+2][x-1] == 0) {  //   up    -> left
        space.x = x-1;
        space.y = y+2;
        available_moves[moves] = space;
        moves++;
    }
    if (x+1<cols  &&  y-2>=0    &&  board[y-2][x+1] == 0) {  //   down  -> right
        space.x = x+1;
        space.y = y-2;
        available_moves[moves] = space;
        moves++;
    }
    if (x-1>=0    &&  y-2>=0    &&  board[y-2][x-1] == 0) {  //   down  -> left
        space.x = x-1;
        space.y = y-2;
        available_moves[moves] = space;
        moves++;
    }


    if (moves > 1) {
        printf("THREAD %u: %u moves possible after move #%u; creating threads\n",(unsigned int)pthread_self(), moves, visited);
        fflush(stdout);
    }

    return moves;
}

// POPULATE BOARD ================================================================================
/* Populate a newly created board with the already visited spaces of the previous board         */
void populate_board(int** new_board, int** old_board) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            new_board[i][j] = old_board[i][j];
        }
    }
}

// TAKE THE TOUR =================================================================================
/* Knights tour control function  (thread function)                                             */
void* take_the_tour(void* args) {

    TA_t* thread_args = (TA_t*)args;
    int visited = thread_args->visited;
    position_t knight;
    knight.x = thread_args->current_move.x;
    knight.y = thread_args->current_move.y;

    // build a new board, populate with the already closed spaces, and close the new space
    int** board = matrix_alloc();
    populate_board(board, thread_args->board);
    board[knight.y][knight.x] = 1;


    pthread_t* tid = malloc(8 * sizeof(pthread_t));
    unsigned int moves = 0;
    unsigned int attempts = 0;
    unsigned int num_children = 0;
    position_t available_moves[8] = {0};

    TA_t* send_args = malloc(8 * sizeof(TA_t)); // arguments to send to possible child threads

    // knights have at most eight legal moves
    while (attempts < 8) {

        // on the first loop, search for possible moves and set up pipe for all children
        if (attempts == 0) {
            moves = num_possible_moves(board, available_moves, knight, visited);
        }

        // no moves found
        if (moves == 0) {
            free(tid);
            dead_end(board, visited);   // thread exits

        // make a move, but don't spawn a new thread
        } else if (moves == 1) {
            knight.x = available_moves[0].x;
            knight.y = available_moves[0].y;
            attempts = 0;
            moves = 0;
            visited++;
            board[knight.y][knight.x] = 1;

        // spawn threads to explore multiple moves
        } else {
            num_children = moves;

            for (int i = 0; i < moves; i++) {
                send_args[i].board = board;
                send_args[i].current_move.x = available_moves[i].x;
                send_args[i].current_move.y = available_moves[i].y;
                send_args[i].visited = visited+1;

                // New thread
                if ( pthread_create( &tid[i], NULL, take_the_tour, (void*)&send_args[i] ) != 0 ) {
                    fprintf( stderr, "ERROR: Could not create thread\n" );
                    exit(EXIT_FAILURE);
                }
            }

            break;  // after creating new threads, stop searching and wait for children
        }
    }


    // Join child threads
    unsigned int* x;
    for (int i = 0; i < num_children; i++) {
        if (pthread_join( tid[i], (void **)&x )!= 0 )
            fprintf( stderr, "MAIN: Could not join thread\n");

        free(x);
    }

    free(send_args);
    free(tid);

    // master thread still has more work to do, all other threads exit
    if (pthread_self() != master_thread) {
        if (num_children > 0)
            matrix_free(board);

        unsigned int * y = malloc( sizeof( unsigned int ) );
        *y = pthread_self();
        pthread_exit( y );
    }

    // only the master thread reaches this
    matrix_free(board);
    return NULL;
}



// MAIN ========================================================================================
int main(int argc, char** argv){

    // Argument checking
    if (argc < 3 || argc > 4 || (argc == 4 && ((atoi(argv[3]) > atoi(argv[1])*atoi(argv[2])) || atoi(argv[3]) < 0)) ||  atoi(argv[1]) <= 2 || atoi(argv[2]) <= 2) {
        fprintf(stderr, "ERROR: Invalid argument(s)\n");
        fprintf(stderr, "USAGE: a.out <m> <n> [<k>]\n");
        return EXIT_FAILURE;

    } else if (atoi(argv[1]) * atoi(argv[2]) > 20) {
        fprintf(stderr, "ERROR: Too expensive\n");
        return EXIT_FAILURE;
    }

    // assigning global variable
    rows = atoi(argv[1]);
    cols = atoi(argv[2]);
    available_spaces = rows * cols;
    dead_end_boards = (int***)malloc(size_DEB * sizeof(int**));
    master_thread = pthread_self();

    if (argc == 4)
        knight_visits = atoi(argv[3]);

    int** initial_board = matrix_alloc();
    printf("THREAD %u: Solving the knight's tour problem for a %dx%d board\n", (unsigned int)pthread_self(), rows, cols);

    // Take the tour is the thread function which is the reason for the odd variable passing
    TA_t thread_args;
    thread_args.visited = 1;
    thread_args.board = initial_board;
    thread_args.current_move.x = 0;         // knight always starts in the top left corner (0,0)
    thread_args.current_move.y = 0;


    // MAIN control function --------------------------
    take_the_tour((void*)&thread_args);
    matrix_free(initial_board);

    printf("THREAD %u: Best solution found visits %u squares (out of %u)\n", (unsigned int)pthread_self(), max_squares, available_spaces);

    // Free and print dead end boards
    printf("THREAD %u: There are %d total dead end boards:\n", (unsigned int)pthread_self(), index_DEB);
    for (int i = 0; i < index_DEB; i++) {
        #ifdef PRINT_BOARDS
            print_board(dead_end_boards[i]);
        #endif
        matrix_free(dead_end_boards[i]);
    }

    return EXIT_SUCCESS;
}
