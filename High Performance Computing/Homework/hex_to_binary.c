#include <stdlib.h>
#include <stdio.h>

#define HEX_DIGITS 4
#define BITS HEX_DIGITS*4


void add_bits(unsigned int* binary_array, int dig1, int dig2, int dig3, int dig4);
void print_binary_array(unsigned int* array, int size);

// MAIN ================================================
int main (void) {

    printf("Please enter a %d-bit hexadecimal number:\n", HEX_DIGITS);
    char hex[HEX_DIGITS] = {0};

    scanf("%s", hex);

    printf("You entered: %s\n", hex);
    printf("The equivalent binary is:\n");

    unsigned int binary_array[BITS] = {0};

    int i = 0;
    while (hex[i]) {
        switch (hex[i]) {
            case '0':
                add_bits(binary_array, 0,0,0,0); break;
            case '1':
                add_bits(binary_array, 0,0,0,1); break;
            case '2':
                add_bits(binary_array, 0,0,1,0); break;
            case '3':
                add_bits(binary_array, 0,0,1,1); break;
            case '4':
                add_bits(binary_array, 0,1,0,0); break;
            case '5':
                add_bits(binary_array, 0,1,0,1); break;
            case '6':
                add_bits(binary_array, 0,1,1,0); break;
            case '7':
                add_bits(binary_array, 0,1,1,1); break;
            case '8':
                add_bits(binary_array, 1,0,0,0); break;
            case '9':
                add_bits(binary_array, 1,0,0,1); break;
            case 'A':
                add_bits(binary_array, 1,0,1,0); break;
            case 'B':
                add_bits(binary_array, 1,0,1,1); break;
            case 'C':
                add_bits(binary_array, 1,1,0,0); break;
            case 'D':
                add_bits(binary_array, 1,1,0,1); break;
            case 'E':
                add_bits(binary_array, 1,1,1,0); break;
            case 'F':
                add_bits(binary_array, 1,1,1,1); break;
            case 'a':
                add_bits(binary_array, 1,0,1,0); break;
            case 'b':
                add_bits(binary_array, 1,0,1,1); break;
            case 'c':
                add_bits(binary_array, 1,1,0,0); break;
            case 'd':
                add_bits(binary_array, 1,1,0,1); break;
            case 'e':
                add_bits(binary_array, 1,1,1,0); break;
            case 'f':
                add_bits(binary_array, 1,1,1,1); break;
            default:
                printf("\n Invalid hexa digit %c ", hex[i]);
        }

        ++i;
    }

    print_binary_array(binary_array, HEX_DIGITS*4);


    return 0;
}

// ADD BITS: append bits to the binary array ===============================
void add_bits(unsigned int* binary_array, int dig1, int dig2, int dig3, int dig4) {
    static int index = BITS-1;

    binary_array[index] = dig1;
    binary_array[index-1] = dig2;
    binary_array[index-2] = dig3;
    binary_array[index-3] = dig4;

    index -= 4;

}

// PRINT BINARY ARRAY =======================================================
void print_binary_array(unsigned int* array, int size){

    for (int i = size-1; i >= 0; i--){
        printf("%d", array[i]);
    }
}
