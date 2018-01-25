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

// READ DIRECTORY CONTENTS ===============================================================
void read_directory_contents(DIR* directory, alnum_t* dictionary) {
    struct dirent* dir_pt;
    while ((dir_pt = readdir(directory)) != NULL){

    }

    closedir(directory);
}





// MAIN =================================================================================
int main(int argc, char** argv){
    // Ensure proper usage
    if (argc < 2){
        fprintf(stderr, "USAGE: %s <working_directory>  OPTIONAL: <num_output_words>\n", argv[0]);
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

    // Attempt to open the specified directory
    char* filepath = argv[1];
    DIR* directory = opendir(filepath);

    if (directory == NULL){
        perror("Cannot open");
        return EXIT_FAILURE;
    }

    read_directory_contents(directory, dictionary);

    free(dictionary);
    dictionary = NULL;

	return EXIT_SUCCESS;
}
