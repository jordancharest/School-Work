#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

#define WORD_SIZE 80
#define INITIAL_SIZE 32

// Variable declarations
typedef struct alphanumeric
{
    unsigned int count;
    char word[WORD_SIZE];

} alnum_t;

unsigned int word_count = 0;
unsigned int space_avail = INITIAL_SIZE;



// MAIN =================================================================================
int main(int argc, char** argv){
    // Ensure proper usage
    if (argc < 2){
        fprintf(stderr, "USAGE: %s <working_directory>  OPTIONAL: <num_output_words>", argv[0]);
        return EXIT_FAILURE;
    }


    // Dictionary of word counts
    alnum_t* dictionary = malloc(INITIAL_SIZE * (sizeof(unsigned int) + WORD_SIZE));
    if ( dictionary == NULL )
    {
        fprintf( stderr, "malloc() failed" );
        return EXIT_FAILURE;
    }

    printf("Allocated initial struct array of size %d.\n", INITIAL_SIZE);

    // Attempt to open the directory
    char* filepath = argv[1];
    DIR* directory = opendir(filepath);
    if ( directory == NULL){
        perror("Cannot open %s", filepath);
        return EXIT_FAILURE;
    }

    // Begin reading directory contents
    while ((dir_pointer = readdir(directory)) != NULL){

    }

    closedir(directory);




















    free(dictionary);
    dictionary = NULL;

	return EXIT_SUCCESS;
}
