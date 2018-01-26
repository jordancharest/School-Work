#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>

#define WORD_LENGTH 80
#define INITIAL_SIZE 32

// Variable declarations
typedef struct alphanumeric
{
    unsigned int count;
    char word[WORD_LENGTH];

} alnum_t;

unsigned int word_count = 0;
unsigned int space_available = INITIAL_SIZE;
const unsigned int DICT_ENTRY_SIZE = sizeof(unsigned int) + WORD_LENGTH;


// INCREASE DICTIONARY_SIZE ==============================================================
/* The dictionary has run out of space. Double the size of allocated memory.            */
alnum_t* increase_dictionary_size(alnum_t* dictionary){

    printf("Attempting reallocation...\n");

    space_available *= 2;
    dictionary = (alnum_t*)realloc( dictionary, space_available * DICT_ENTRY_SIZE );
    if (dictionary == NULL){
        fprintf(stderr, "realloc failed()");
    }
    printf("Re-allocated struct array to be size %d.\n", space_available);

    return dictionary;
}



// VERIFY ALPHANUMERIC ===================================================================
/* Verifies whether a word read by fscanf is alphanumeric in order to add to the
    dictionary. If it is not, searches for punctuation to possibly separate into two
    separate words.                                                                     */

void verify_alphanumeric(alnum_t* dictionary, char* new_word){
    //printf("Verifying if word is alphanumeric\n");

    int i = 0;
    bool valid = true;
    while(new_word[i] != '\0'){
        if(isalnum(new_word[i])) {
            i++;
        } else {
            printf("Character %c is not alnum\n", new_word[i]);
            valid = false;
            break;
        }
    }

    if (valid){  // add the word to the dictionary
        printf("Adding %s\n", new_word);
        strcpy(dictionary[word_count].word, new_word);
        dictionary[word_count].count = 1;
        word_count++;
        //return;
    }

    // Perform other checks such as for contractions





}


// LINEAR SEARCH =========================================================================
/* Perform a linear search of the dictionary struct for the presence of the candidate
    word. Return index of word instance.                                                */
unsigned int lin_search(alnum_t* dictionary, char* new_word){
    //printf("Attempting to find word in dictionary\n");
    for (int i = 0; i < word_count; i++){
        if (strcmp(new_word, dictionary[i].word) == 0){
            printf("%s equivalent to word at index %d\n", new_word, i);
            return i;
        }
    }

    return word_count;
}



// READ FILE CONTENTS ====================================================================
/* Opens the file and reads all alphanumeric words */
alnum_t* read_file_contents(char* full_path, alnum_t* dictionary){
    printf("Found regular file. Attempting to read...\n");

    FILE* file = fopen(full_path, "r");
    if (file == NULL){
        perror ("fopen() failed");
        exit(EXIT_FAILURE);
    }

    char new_word[WORD_LENGTH];
    int index;
    // read a word, check if it is in the dictionary,
    // add word / increase word count accordingly
    while ((fscanf(file, "%s", new_word)) != EOF){

        if (word_count == space_available-1){
            dictionary = increase_dictionary_size(dictionary);
        }
        printf("%d %d\n", word_count, space_available);

        index = lin_search(dictionary, new_word);
        //printf("Lin_search returned index %d\n", index);
        //printf("the current word_count is %d\n", word_count);

        if (index < word_count){        // the word was already in the dictionary
            //printf("Word already found in dictionary...\n");
            dictionary[index].count++;
        } else {
            verify_alphanumeric(dictionary, new_word);
        }
    }

    fclose(file);

    // may be a different pointer than was sent if reallocation occurred
    return dictionary;
}



// READ DIRECTORY CONTENTS ===============================================================
/* Loops through the specified directory checking for regular files. Any regular files
    are passed to read_file_contents() to be read.                                      */
alnum_t* read_directory_contents(DIR* directory, alnum_t* dictionary, char* file_path) {

    struct dirent* file;
    struct stat filedata;
    char full_path[NAME_MAX];   // NAME_MAX: macro for maximum file path length, typically 1 byte

    while ((file = readdir(directory)) != NULL){
        printf("\nThe current file is: %s\n", file->d_name);
        sprintf(full_path, "%s/%s", file_path, file->d_name);

        // get file data for current file
        int st_err = lstat(full_path, &filedata);
        if (st_err == -1) {
                perror("Failed to read file data");
                exit(EXIT_FAILURE);
        }

        // check if the current file is a regular file
        if (S_ISREG(filedata.st_mode)){
            dictionary = read_file_contents(full_path, dictionary);

        } else {    // ignore if it is not a regular file
            printf("  Not a regular file\n\n");
            continue;
        }
    }

    closedir(directory);

    // may be a different pointer than was sent if reallocation occurred
    return dictionary;
}


// SUMMARY ===============================================================================
void summary(alnum_t* dictionary){





}



// MAIN ==================================================================================
int main(int argc, char** argv){
    // Ensure proper usage
    if (argc < 2){
        fprintf(stderr, "USAGE: %s <working_directory>  OPTIONAL: <num_output_words>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Dictionary of word counts
    alnum_t* dictionary = malloc(INITIAL_SIZE * DICT_ENTRY_SIZE);
    if ( dictionary == NULL )
    {
        fprintf( stderr, "malloc() failed\n" );
        return EXIT_FAILURE;
    }

    printf("Allocated initial struct array of size %d.\n", INITIAL_SIZE);

    // Attempt to open the specified directory
    char* file_path = argv[1];
    printf("Current Working Directory: %s\n\n", file_path);
    DIR* directory = opendir(file_path);

    if (directory == NULL){
        perror("Cannot open directory");
        return EXIT_FAILURE;
    }

    dictionary = read_directory_contents(directory, dictionary, file_path);

    summary(dictionary);

    free(dictionary);
    dictionary = NULL;

	return EXIT_SUCCESS;
}
