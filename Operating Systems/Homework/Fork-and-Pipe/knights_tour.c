#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef struct board_position {
    unsigned int x;
    unsigned int y;
} position_t;

int available_spaces = 0;
int rows = 0;
int cols = 0;
// PRINT BOARD ===================================================================================
void print_board(int** board) {
    for (int i = 0; i < rows; i++) {
        printf("PID %d:   ", getpid());
        for (int j = 0; j < cols; j++) {
            if (board[i][j]) printf("k");
            else printf(".");
        }
        printf("\n");
    }
}

// DEAD END ======================================================================================
void dead_end(int* pipes, int write_index, int visited) {
    int bytes_written = write( pipes[write_index], &visited, sizeof(visited));
    printf("PID %d: Dead end after move #%d\n", getpid(), visited);
    printf("PID %d: Sent %d on pipe to parent - FD: %d\n", getpid(), visited, pipes[write_index]);

    if (bytes_written != sizeof(visited)) {
        fprintf(stderr, "ERROR: write() failed");
        exit(EXIT_FAILURE);
    }
    #ifdef DISPLAY_BOARD
        print_board(board);
    #endif
}



// NUMBER OF POSSIBLE MOVES ======================================================================
int num_possible_moves(int** board, position_t* available_moves, position_t knight, int visited) {
    int moves = 0;
    int x = knight.x;
    int y = knight.y;
    position_t space = {0,0};
                                                             // 2 spaces | 1 space
    if (x+2<cols  &&  y+1<rows  &&  board[x+2][y+1] == 0) {  //   right -> up
        space.x = x+2;
        space.y = y+1;
        available_moves[moves] = space;
        moves++;
    }
    if (x+2<cols  &&  y-1>=0    &&  board[x+2][y-1] == 0) {  //   right -> down
        space.x = x+2;
        space.y = y-1;
        available_moves[moves] = space;
        moves++;
    }
    if (x-2>=0    &&  y+1<rows  &&  board[x-2][y+1] == 0) {  //   left  -> up
        space.x = x-2;
        space.y = y+1;
        available_moves[moves] = space;
        moves++;
    }
    if (x-2>=0    &&  y-1>=0    &&  board[x-2][y-1] == 0) {  //   left  -> down
        space.x = x-2;
        space.y = y-1;
        available_moves[moves] = space;
        moves++;
    }
    if (x+1<cols  &&  y+2<rows  &&  board[x+1][y+2] == 0) {  //   up    -> right
        space.x = x+1;
        space.y = y+2;
        available_moves[moves] = space;
        moves++;
    }
    if (x-1>=0    &&  y+2<rows  &&  board[x-1][y+2] == 0) {  //   up    -> left
        space.x = x-1;
        space.y = y+2;
        available_moves[moves] = space;
        moves++;
    }
    if (x+1<cols  &&  y-2>=0    &&  board[x+1][y-2] == 0) {  //   down  -> right
        space.x = x+1;
        space.y = y-2;
        available_moves[moves] = space;
        moves++;
    }
    if (x-1>=0    &&  y-2>=0    &&  board[x-1][y-2] == 0) {  //   down  -> left
        space.x = x-1;
        space.y = y-2;
        available_moves[moves] = space;
        moves++;
    }
    printf("PID %d: %d moves possible after move #%d\n", getpid(), moves, visited);
#ifdef DISPLAY_BOARD
    print_board(board);
#endif
    return moves;
}

// REAP CHILDREN =================================================================================
/* Gather data from all children to determine if a tour was completed                           */
void reap_children(int* pipes, int read_index, int write_index, int children, pid_t master_pid) {
    printf("PID %d: REAPING %d CHILDREN\n", getpid(), children);

    int status;
    int child_path;
    int max_path = 0;
    int bytes_read;
    int bytes_written;
    pid_t child_pid;

    while (children > 0) {
        child_pid = wait(&status);
        children--;
        if ( WIFSIGNALED(status) ){
            fprintf( stderr, "ERROR: child process %d terminated abnormally\n", child_pid );  /* core dump or kill or kill -9 */
            exit(EXIT_FAILURE);
        }

        bytes_read = read(pipes[read_index], &child_path, sizeof(child_path));
        if(bytes_read != sizeof(child_path)){
            fprintf(stderr, "ERROR: write() failed");
            exit(EXIT_FAILURE);
        }
        printf("PID %d: Received %d from child - FD: %d\n", getpid(), child_path, pipes[read_index]);
        if (child_path > max_path) max_path = child_path;

        if (children == 0 && master_pid != getpid()){
            bytes_written = write( pipes[write_index], &max_path, sizeof(max_path));
            if(bytes_written != sizeof(max_path)){
                fprintf(stderr, "ERROR: write() failed");
                exit(EXIT_FAILURE);
            }
            printf("PID %d: All child processes terminated; sent %d on pipe to parent - FD: %d\n", getpid(), max_path, pipes[write_index]);
        }
    }

    if (master_pid == getpid()) {
        printf("PID %d: Best solution found visits %d squares (out of %d)\n", master_pid, max_path, available_spaces);
    }
}



// TAKE THE TOUR =================================================================================
void take_the_tour(int** board) {
    int visited = 1;
    int moves = 0;
    int attempts = 0;
    int children = 0;
    int parent = 0;
    int total_children = 0;
    position_t knight = {0,0};
    position_t available_moves[8] = {0};

    pid_t pid_rc;
    int pipes[80];      // whole lotta pipes
    int read_index = 0;    // each process should save its read file descriptor from the array of pipes
    int write_index = -1;

    pid_t master_pid = getpid();

    // knights have at most eight legal moves
    while (attempts < 8) {
        // on the first loop, search for possible moves and set up pipe for all children
        if (attempts == 0) {
            moves = num_possible_moves(board, available_moves, knight, visited);
            int rc = pipe( pipes + read_index );
            if ( rc == -1 ) {
                perror( "pipe() failed" );
                exit(EXIT_FAILURE);
            }
        }

        if (moves == 0) {
            dead_end(pipes, write_index, visited);
            break;
        }

        for (int i = 0; i < moves; i++) {
            pid_rc = fork();
            if ( pid_rc == -1 ) {
                perror( "fork() failed" );
                exit(EXIT_FAILURE);
            }

            if (pid_rc == 0) {          /* CHILD PROCESS */
                read_index += 2;        // child reads from and writes to a different pipe than the parent
                write_index += 2;
                knight.x = available_moves[i].x;      // update the knights location
                knight.y = available_moves[i].y;
                attempts = 0;
                moves = 0;
                children = 0;
                parent = 0;
                total_children++;
                visited++;
                for (int i = 0; i < 8; i++) {
                    available_moves[i].x = 0;
                    available_moves[i].y = 0;
                }

                board[knight.x][knight.y] = 1;
                break;

            } else {                    /* PARENT PROCESS */
                children++;
                attempts++;
                parent = 1;
            }
        }
        if (parent == 1) break;
    }

    reap_children(pipes, read_index, write_index, children, master_pid);
}

// MATRIX ALLOCATION =============================================================================
/* allocate an array of pointers to ints, then allocate a row/array of ints and assign each
    int pointer that row (ROW MAJOR)                                                            */
int** matrix_alloc() {
    int **array = (int **)calloc(rows, sizeof(int*));
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





// MAIN ========================================================================================
int main(int argc, char** argv){

    if (argc != 3) {
        fprintf(stderr, "ERROR: Invalid argument(s)\n");
        fprintf(stderr, "USAGE: %s <rows> <columns>\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (atoi(argv[1]) > 6 || atoi(argv[2]) > 6) {
        fprintf(stderr, "ERROR: Too expensive\n");
        return EXIT_FAILURE;
    }

    rows = atoi(argv[1]);
    cols = atoi(argv[2]);
    available_spaces = rows * cols;

    int** board = matrix_alloc(rows, cols);
    printf("PID %d: Solving the knight's tour problem for a %dx%d board\n", getpid(), rows, cols);

    // knight always starts in the top left corner
    board[0][0] = 1;

    take_the_tour(board);
    matrix_free(board);

    return EXIT_SUCCESS;
}
