#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

int available_spaces = 0;
int rows = 0;
int cols = 0;

// PRINT BOARD ===================================================================================
void print_board(int** board) {
    for (int i = 0; i < rows; i++) {
        printf("PID %d:   ", getpid());
        for (int j = 0; j < cols; j++) {
            printf("%d", board[i][j]);
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


// MAIN ========================================================================================
int main(int argc, char** argv){

    if (argc != 3) {
        fprintf(stderr, "ERROR: Invalid argument(s)\n");
        fprintf(stderr, "USAGE: %s <rows> <columns>\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (atoi(argv[1]) * atoi(argv[2]) > 20) {
        fprintf(stderr, "ERROR: Too expensive\n");
        return EXIT_FAILURE;
    }

    rows = atoi(argv[1]);
    cols = atoi(argv[2]);
    available_spaces = rows * cols;

    int** board = matrix_alloc();
    printf("PID %d: Solving the knight's tour problem for a %dx%d board\n", getpid(), rows, cols);
    print_board(board);

    board[rows-1][0] = 5;
    board[0][cols-1] = 6;

    printf("\nRow end: %d\n", board[rows-1][0]);
    printf("\nCol end: %d\n", board[0][cols-1]);

    // knight always starts in the top left corner
    board[0][0] = 1;
    print_board(board);

    matrix_free(board);

    return EXIT_SUCCESS;
}
