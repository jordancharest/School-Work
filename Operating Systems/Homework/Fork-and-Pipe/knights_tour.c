#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

typedef struct board_position {
    unsigned int x;
    unsigned int y;
} position_t;

int available_spaces = 0;
int rows = 0;
int cols = 0;

// NUMBER OF POSSIBLE MOVES ======================================================================
void num_possible_moves(int** board, position_t knight, int visited) {
    int moves = 0;
    int x = knight.x;
    int y = knight.y;
                                                                   // 2 spaces | 1 space
    if (x+2<cols  &&  y+1<rows  &&  board[x+2][y+1] == 0) moves++; //   right -> up
    if (x+2<cols  &&  y-1>=0    &&  board[x+2][y-1] == 0) moves++; //   right -> down
    if (x-2>=0    &&  y+1<rows  &&  board[x-2][y+1] == 0) moves++; //   left  -> up
    if (x-2>=0    &&  y-1>=0    &&  board[x-2][y-1] == 0) moves++; //   left  -> down
    if (x+1<cols  &&  y+2<rows  &&  board[x+1][y+2] == 0) moves++; //   up    -> right
    if (x-1>=0    &&  y+2<rows  &&  board[x-1][y+2] == 0) moves++; //   up    -> left
    if (x+1<cols  &&  y-2>=0    &&  board[x+1][y-2] == 0) moves++; //   down  -> right
    if (x-1>=0    &&  y-2>=0    &&  board[x-1][y-2] == 0) moves++; //   down  -> left

    printf("PID %d: %d moves possible after move #%d\n", getpid(), moves, visited);
}

// FIND OPEN SPACE ===============================================================================
/* Searches the space of legal knight moves to determine if any of them are unvisited           */
position_t find_open_space(int** board, position_t knight) {
    position_t space = {0, 0};
    int x = knight.x;
    int y = knight.y;
                                                                    // 2 spaces | 1 space
    if (x+2<cols  &&  y+1<rows  &&  board[x+2][y+1] == 0) {         //   right -> up
        space.x = x+2;
        space.y = y+1;
    } else if (x+2<cols  &&  y-1>=0    &&  board[x+2][y-1] == 0) {  //   right -> down
        space.x = x+2;
        space.y = y-1;
    } else if (x-2>=0    &&  y+1<rows  &&  board[x-2][y+1] == 0) {  //   left  -> up
        space.x = x-2;
        space.y = y+1;
    } else if (x-2>=0    &&  y-1>=0    &&  board[x-2][y-1] == 0) {  //   left  -> down
        space.x = x-2;
        space.y = y-1;
    } else if (x+1<cols  &&  y+2<rows  &&  board[x+1][y+2] == 0) {  //   up    -> right
        space.x = x+1;
        space.y = y+2;
    } else if (x-1>=0    &&  y+2<rows  &&  board[x-1][y+2] == 0) {  //   up    -> left
        space.x = x-1;
        space.y = y+2;
    } else if (x+1<cols  &&  y-2>=0    &&  board[x+1][y-2] == 0) {  //   down  -> right
        space.x = x+1;
        space.y = y-2;
    } else if (x-1>=0    &&  y-2>=0    &&  board[x-1][y-2] == 0) {  //   down  -> left
        space.x = x-1;
        space.y = y-2;
    }

    return space;
}

// REAP CHILDREN =================================================================================
/* Gather data from all children to determine if a tour was completed                           */
void reap_children(int* pipes, int read_fd, int children) {

}



// TAKE THE TOUR =================================================================================
void take_the_tour(int** board) {
    int visited = 1;
    int attempts = 0;
    int children = 0;
    position_t knight = {0,0};
    position_t next = {0,0};

    pid_t pid_rc;
    int pipes[80];      // whole lotta pipes
    int read_fd = 0;    // each process should save its read file descriptor from the array of pipes
    int write_fd = 1;

    // knights have at most eight legal moves
    while (attempts < 8) {
        // on the first attempt, output number of possible moves and set up pipe for all children
        if (attempts == 0) {
            num_possible_moves(board, knight, visited);
            int rc = pipe( pipes + read_fd );
            if ( rc == -1 ) {
                perror( "pipe() failed" );
                exit(EXIT_FAILURE);
            }
            close(pipes[read_fd+1]);    // parent will never write to the pipe
            read_fd += 2;               // prepare for next pipe creation
        }

        next = find_open_space(board, knight);
        if (next.x == 0 && next.y == 0) {
            printf("PID %d: Dead end after move #%d\n", getpid(), visited);
            printf("PID %d: Sent %d on pipe to parent", getpid(), visited);
            write( pipes[write_fd], &visited, sizeof(visited));
            break;

        // legal move found, let another process explore it
        } else {
            pid_rc = fork();
            if ( pid_rc == -1 )
            {
                perror( "fork() failed" );
                exit(EXIT_FAILURE);
            }

            if (pid_rc == 0) {          /* CHILD PROCESS */
                write_fd += 2;          // child writes to a different pipe than the parent
                knight.x = next.x;      // update the knights location
                knight.y = next.y;
                attempts = 0;
                visited++;

            } else {                    /* PARENT PROCESS */
                printf("PID %d: Child proces created\n", getpid());
                children++;


            }


        }


        attempts++;
    }

    reap_children(pipes, read_fd, children);
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





// MAIN ========================================================================================
int main(int argc, char** argv){

    if (argc != 3) {
        fprintf(stderr, "ERROR: Invalid argument(s)\n");
        fprintf(stderr, "USAGE: %s <rows> <columns>\n", argv[0]);
        return EXIT_FAILURE;
    } else if (atoi(argv[1]) > 4 || atoi(argv[2]) > 4) {
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
    free(board);
    board = NULL;

    return EXIT_SUCCESS;
}
