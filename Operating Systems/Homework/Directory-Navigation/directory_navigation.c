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

unsigned int unique_words = 0;
unsigned int total_words = 0;
unsigned int space_available = INITIAL_SIZE;
const unsigned int DICT_ENTRY_SIZE = sizeof(unsigned int) + WORD_LENGTH;


// INCREASE DICTIONARY_SIZE ==============================================================
/* The dictionary has run out of space. Double the size of allocated memory.            */
alnum_t* increase_dictionary_size(alnum_t* dictionary){
    space_available += INITIAL_SIZE;

    dictionary = (alnum_t*)realloc( dictionary, space_available * DICT_ENTRY_SIZE );
    if (dictionary == NULL){
        fprintf(stderr, "ERROR: realloc failed()");
    }
    printf("Re-allocated struct array to be size %d.\n", space_available);

    return dictionary;
}


// LINEAR SEARCH =========================================================================
/* Perform a linear search of the dictionary struct for the presence of the candidate
    word. Return index of word instance or end of array if word is not present.         */
unsigned int lin_search(alnum_t* dictionary, char* new_word){
    for (int i = 0; i < unique_words; i++){
        if (strcmp(new_word, dictionary[i].word) == 0){
            return i;
        }
    }

    return unique_words;
}



// READ FILE CONTENTS ====================================================================
/* Reads all alphanumeric 'words' from the given file                                   */
alnum_t* read_file_contents(char* full_path, alnum_t* dictionary){

    FILE* file = fopen(full_path, "r");
    if (file == NULL){
        perror ("ERROR: fopen() failed");
        exit(EXIT_FAILURE);
    }

    char new_word[WORD_LENGTH];
    char new_char;
    int index;
    int num_letters = 0;

    // one character at a time
    while ((new_char = fgetc(file)) != EOF){
        if (unique_words == space_available){
            dictionary = increase_dictionary_size(dictionary);
        }

        // check if we can add another character to the end of the current word
        if (isalnum(new_char)){
            new_word[num_letters] = new_char;
            num_letters++;
            continue;

        // if not, check if we have already completed a word
        } else if (num_letters > 1) {
            new_word[num_letters] = '\0';   // null terminate the current word
            num_letters = 0;

        /* the current character is not alphanumeric and we cannot create a word
            with the previous letters; scrap it and move on                     */
        } else {
            new_word[0] = '\0';
            num_letters = 0;
            continue;
        }

        // below is executed every time a word is completed --------------------------
        total_words++;
        index = lin_search(dictionary, new_word);

        if (index < unique_words){        // the word was already in the dictionary
            dictionary[index].count++;
        } else {
            strcpy(dictionary[unique_words].word, new_word);
            dictionary[unique_words].count = 1;
            unique_words++;
        }
    }
    fclose(file);

    // may be a different pointer than the one sent if reallocation occurred
    return dictionary;
}



// READ DIRECTORY CONTENTS ===============================================================
/* Loops through the specified directory checking for regular files. Any regular files
    are passed to read_file_contents() to be read.                                      */
alnum_t* read_directory_contents(DIR* directory, alnum_t* dictionary, char* file_path) {

    struct dirent* file;
    struct stat filedata;
    char full_path[NAME_MAX];   // NAME_MAX: macro for max file path length, usually 1 byte

    while ((file = readdir(directory)) != NULL){
        sprintf(full_path, "%s/%s", file_path, file->d_name);

        // get file data for current file
        int st_err = lstat(full_path, &filedata);
        if (st_err == -1) {
                perror("ERROR: failed to read file data");
                exit(EXIT_FAILURE);
        }

        // attempt to read contents if the current file is a regular file
        if (S_ISREG(filedata.st_mode)) dictionary = read_file_contents(full_path, dictionary);
    }

    closedir(directory);

    // may be a different pointer than was sent if reallocation occurred
    return dictionary;
}


// SUMMARY ===============================================================================
void print_dictionary(alnum_t* dictionary, int num_display){
    if (num_display != 0 && num_display < unique_words){
        printf("First %d words ", num_display);
    } else {
        num_display = 0;
        printf("All words ");
    }
    printf("(and corresponding counts) are:\n");

    if (num_display == 0){
        for (int i = 0; i < unique_words; i++){
            printf("%s -- %d\n", dictionary[i].word, dictionary[i].count);
        }
    } else {
        for (int i = 0; i < num_display; i++){
            printf("%s -- %d\n", dictionary[i].word, dictionary[i].count);
        }
        printf("Last %d words (and corresponding counts) are:\n", num_display);
        for (int i = num_display; i >0; i--){
            printf("%s -- %d\n", dictionary[unique_words-i].word, dictionary[unique_words-i].count);
        }
    }
}



// MAIN ==================================================================================
int main(int argc, char** argv){
    int num_display = 0;

    // Ensure proper usage
    if (argc < 2){
        fprintf(stderr, "ERROR: not enough command line arguments\n");
        fprintf(stderr, "USAGE: %s <search_directory>  OPTIONAL: <num_output_words>\n", argv[0]);
        return EXIT_FAILURE;
    } else if (argc == 3){
        num_display = atoi(argv[2]);
    }

    // Dictionary of word counts
    alnum_t* dictionary = malloc(INITIAL_SIZE * DICT_ENTRY_SIZE);
    if ( dictionary == NULL ){
        fprintf( stderr, "ERROR: malloc() failed\n" );
        return EXIT_FAILURE;
    }

    printf("Allocated initial struct array of size %d.\n", INITIAL_SIZE);

    // Attempt to open the specified directory
    char* file_path = argv[1];
    DIR* directory = opendir(file_path);

    if (directory == NULL){
        perror("ERROR: cannot open directory");
        return EXIT_FAILURE;
    }

    dictionary = read_directory_contents(directory, dictionary, file_path);

    printf("All done (successfully read %d words; %d unique words).\n", total_words, unique_words);
    print_dictionary(dictionary, num_display);

    free(dictionary);
    dictionary = NULL;

    return EXIT_SUCCESS;
}
