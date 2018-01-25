#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
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
void read_directory_contents(DIR* directory, alnum_t* dictionary, char* filepath) {
    struct dirent* dir_pt;
    struct stat filedata;
    char full_path[NAME_MAX];

    while ((dir_pt = readdir(directory)) != NULL){
        printf("The current file is: %s\n", dir_pt->d_name);
        sprintf(full_path, "%s/%s", filepath, dir_pt->d_name);


        int st_err = lstat(full_path, &filedata);     // get file data for current file pointed to by dir_pt
        if (st_err == -1) {
                perror("Failed to read file data");
                exit(EXIT_FAILURE);
        }

        // check if the current file is a regular file
        if (S_ISREG(filedata.st_mode)){
            printf("Is regular file\n");
            printf("File size: %ld\n", filedata.st_size);

            //read_file_contents(dictionary);


        } else {
            printf("  Not a regular file\n\n");
            continue;
        }
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
        fprintf( stderr, "malloc() failed\n" );
        return EXIT_FAILURE;
    }

    printf("Allocated initial struct array of size %d.\n", INITIAL_SIZE);

    // Attempt to open the specified directory
    char* filepath = argv[1];
    printf("Current Working Directory: %s\n\n", filepath);
    DIR* directory = opendir(filepath);

    if (directory == NULL){
        perror("Cannot open directory");
        return EXIT_FAILURE;
    }

    read_directory_contents(directory, dictionary, filepath);

    free(dictionary);
    dictionary = NULL;

	return EXIT_SUCCESS;
}
