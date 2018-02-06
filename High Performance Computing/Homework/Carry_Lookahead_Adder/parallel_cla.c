#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define HEX_DIGITS 262144
#define BITS HEX_DIGITS*4
#define BLOCK_SIZE 16


// FUNCTION DECLARATIONS ===================================
void user_input(int* binary_array);
void add_bits(int* binary_array, int index, int dig1, int dig2, int dig3, int dig4);
void print_binary_array(int* array, int size);
void invert_array(int* array, int size);

void bit_level_p_and_g(int* propagate, int* generate, int* A, int* B, int size);
void next_level_P_and_G(int* propagate_next, int* generate_next, int* p, int* g, int size, int block);
void top_level_carry(int* propagate16, int* generate16, int* carry_in16, int size, int block);
void lower_level_carry(int* p, int* g, int* carry_in, int* group_carry, int size, int block);

void carry_lookahead_adder(int* A, int* B, int c_in);
void array_to_hex_string(int* array);
void sum(int* result, int* A, int* B, int* carry_in, int c_in, int size);


// MAIN ====================================================
int main()
{
    // Get two hex numbers to add, convert to binary arrays
    int A[BITS] = {0};
    user_input(A);
    int B[BITS] = {0};
    user_input(B);

    int c_in = 0;

#if 0   // to easily add subtraction later if necessary
    int operation;
    printf("\nAdd (0) or subtract (1):\n");
    scanf("%d", &operation);

    if (operation){      // subtraction
        c_in = 1;
        printf("\nInverting...");
        invert_array(B, BITS);
        printf("\nB (bin) : ");
        print_binary_array(B, BITS);
    }
#endif

    carry_lookahead_adder(A, B, c_in);

    return EXIT_SUCCESS;
}

// USER INPUT ========================================================================
/* Prompt the user for (or scan from file) a hex string, then convert the string
    to an array of 1's and 0's (binary array)                                       */
void user_input(int* binary_array){

    printf("\n\nPlease enter a %d-digit (%d-bit) hexadecimal number:\n", HEX_DIGITS, BITS);
    char hex[HEX_DIGITS] = {0};
    scanf("%s", hex);

    printf("You entered: %s\n", hex);
    printf("The equivalent binary is:\n");

    int i = 0;
    while (hex[i]) {
        switch (hex[i]) {
            case '0':
                add_bits(binary_array, i, 0,0,0,0); break;
            case '1':
                add_bits(binary_array, i, 0,0,0,1); break;
            case '2':
                add_bits(binary_array, i, 0,0,1,0); break;
            case '3':
                add_bits(binary_array, i, 0,0,1,1); break;
            case '4':
                add_bits(binary_array, i, 0,1,0,0); break;
            case '5':
                add_bits(binary_array, i, 0,1,0,1); break;
            case '6':
                add_bits(binary_array, i, 0,1,1,0); break;
            case '7':
                add_bits(binary_array, i, 0,1,1,1); break;
            case '8':
                add_bits(binary_array, i, 1,0,0,0); break;
            case '9':
                add_bits(binary_array, i, 1,0,0,1); break;
            case 'A':
                add_bits(binary_array, i, 1,0,1,0); break;
            case 'B':
                add_bits(binary_array, i, 1,0,1,1); break;
            case 'C':
                add_bits(binary_array, i, 1,1,0,0); break;
            case 'D':
                add_bits(binary_array, i, 1,1,0,1); break;
            case 'E':
                add_bits(binary_array, i, 1,1,1,0); break;
            case 'F':
                add_bits(binary_array, i, 1,1,1,1); break;
            case 'a':
                add_bits(binary_array, i, 1,0,1,0); break;
            case 'b':
                add_bits(binary_array, i, 1,0,1,1); break;
            case 'c':
                add_bits(binary_array, i, 1,1,0,0); break;
            case 'd':
                add_bits(binary_array, i, 1,1,0,1); break;
            case 'e':
                add_bits(binary_array, i, 1,1,1,0); break;
            case 'f':
                add_bits(binary_array, i, 1,1,1,1); break;
            default:
                printf("\n Invalid hex digit %c\n", hex[i]);
        }

        ++i;
    }
    print_binary_array(binary_array, BITS);
}


// ADD BITS ==========================================================================
/* Populate the binary array with the passed digits                                 */
void add_bits(int* binary_array, int index, int dig1, int dig2, int dig3, int dig4) {
    binary_array[BITS - index*4 - 1] = dig1;
    binary_array[BITS - index*4 - 2] = dig2;
    binary_array[BITS - index*4 - 3] = dig3;
    binary_array[BITS - index*4 - 4] = dig4;
}

// PRINT BINARY ARRAY ===============================================================
void print_binary_array(int* array, int size){
    for (int i = size-1; i >= 0; i--){
        if ((i+1)%4 == 0)   printf(" ");
        printf("%d", array[i]);
    }
}

// CARRY LOOKAHEAD ADDER =============================================================
/* Controls the collapses and expansions of the Carry Lookahead Adder algorithm     */
void carry_lookahead_adder(int* A, int* B, int c_in){

    int block = 1;
    int propagate[BITS] = {0};
    int generate[BITS] = {0};
    bit_level_p_and_g(propagate, generate, A, B, BITS);


    // GROUP: Divide into 4 groups, calculate propagate and generate functions for each group
    int group_propagate[BITS/BLOCK_SIZE] = {0};
    int group_generate[BITS/BLOCK_SIZE] = {0};
    block *= BLOCK_SIZE;
    next_level_P_and_G(group_propagate, group_generate, propagate, generate, BITS, block);


    // SECTION
    int section_propagate[ BITS / (BLOCK_SIZE*BLOCK_SIZE) ] = {0};  // using pow() won't compile: 'can't initialize variable sized object'
    int section_generate[BITS/ (BLOCK_SIZE*BLOCK_SIZE) ] = {0};
    block *= BLOCK_SIZE;
    next_level_P_and_G(section_propagate, section_generate, group_propagate, group_generate, BITS, block);


    // SUPER SECTION
    int super_section_propagate[BITS/ (BLOCK_SIZE*BLOCK_SIZE*BLOCK_SIZE) ] = {0};
    int super_section_generate[BITS/ (BLOCK_SIZE*BLOCK_SIZE*BLOCK_SIZE) ] = {0};
    block *= BLOCK_SIZE;
    next_level_P_and_G(super_section_propagate, super_section_generate, section_propagate, section_generate, BITS, block);


    // SUPER SECTION: head back down to bit level, calculating the carry-in along the way
    int super_section_carry[BITS/ (BLOCK_SIZE*BLOCK_SIZE*BLOCK_SIZE)] = {0};
    top_level_carry(super_section_propagate, super_section_generate, super_section_carry, BITS, block);


    // SECTION
    int section_carry[BITS/ (BLOCK_SIZE*BLOCK_SIZE)] = {0};
    lower_level_carry(section_propagate, section_generate, section_carry, super_section_carry, BITS, block);
    block /= BLOCK_SIZE;


    // GROUP
    int group_carry[BITS/BLOCK_SIZE] = {0};      // change if block size changes; should be same size as group_propagate
    lower_level_carry(group_propagate, group_generate, group_carry, section_carry, BITS, block);
    block /= BLOCK_SIZE;


    // BIT LEVEL
    int carry_in[BITS] = {0};
    lower_level_carry(propagate, generate, carry_in, group_carry, BITS, block);


    // CALCULATE RESULT
    int result[BITS] = {0};
    sum(result, A, B, carry_in, c_in, BITS);

#ifdef DEBUG_MODE
    printf("\nBIT Level");
    printf("\nPropagate: ");
    print_binary_array(propagate, BITS);
    printf("\nGenerate:  ");
    print_binary_array(generate, BITS);
    printf("\nCarry:     ");
    print_binary_array(carry_in, BITS);

    printf("\n\nGROUP Level");
    printf("\nPropagate: ");
    print_binary_array(group_propagate, BITS/4);
    printf("\nGenerate:  ");
    print_binary_array(group_generate, BITS/4);
    printf("\nCarry:     ");
    print_binary_array(group_carry, BITS/4);

    printf("\n\nSECTION Level");
    printf("\nPropagate: ");
    print_binary_array(section_propagate, BITS/16);
    printf("\nGenerate:  ");
    print_binary_array(section_generate, BITS/16);
    printf("\nCarry:     ");
    print_binary_array(section_carry, BITS/16);

    printf("\n\nSUPER SECTION Level");
    printf("\nPropagate: ");
    print_binary_array(super_section_propagate, BITS/64);
    printf("\nGenerate:  ");
    print_binary_array(super_section_generate, BITS/64);
    printf("\nCarry:     ");
    print_binary_array(super_section_carry, BITS/64);


    printf("\n\nSUM\n");
    print_binary_array(A, BITS);
    printf("\n");
    print_binary_array(B, BITS);
    printf("\n");
    print_binary_array(carry_in, BITS);
    printf("\n");
    print_binary_array(result, BITS);
    printf("\n\n");
#endif

    array_to_hex_string(result);
}

// ARRAY TO HEX STRING ===================================================================
/* Converts a binary array back to a hex string for output                              */
void array_to_hex_string(int* array){

    char hex[HEX_DIGITS+1];
    hex[HEX_DIGITS] = '\0';

    char temp[5];
    temp[4] = '\0';

    for(int i = (HEX_DIGITS); i > 0; i--){
        // get next four binary digits
        for (int j = 0; j <= 3; j++){
            if (array[4*i-j-1]){
                temp[j] = '1';
            } else {
                temp[j] = '0';
            }
        }

        if (strcmp(temp, "0000") == 0) hex[HEX_DIGITS-i] = '0';
        else if (strcmp(temp, "0001") == 0) hex[HEX_DIGITS-i] = '1';
        else if (strcmp(temp, "0010") == 0) hex[HEX_DIGITS-i] = '2';
        else if (strcmp(temp, "0011") == 0) hex[HEX_DIGITS-i] = '3';
        else if (strcmp(temp, "0100") == 0) hex[HEX_DIGITS-i] = '4';
        else if (strcmp(temp, "0101") == 0) hex[HEX_DIGITS-i] = '5';
        else if (strcmp(temp, "0110") == 0) hex[HEX_DIGITS-i] = '6';
        else if (strcmp(temp, "0111") == 0) hex[HEX_DIGITS-i] = '7';
        else if (strcmp(temp, "1000") == 0) hex[HEX_DIGITS-i] = '8';
        else if (strcmp(temp, "1001") == 0) hex[HEX_DIGITS-i] = '9';
        else if (strcmp(temp, "1010") == 0) hex[HEX_DIGITS-i] = 'A';
        else if (strcmp(temp, "1011") == 0) hex[HEX_DIGITS-i] = 'B';
        else if (strcmp(temp, "1100") == 0) hex[HEX_DIGITS-i] = 'C';
        else if (strcmp(temp, "1101") == 0) hex[HEX_DIGITS-i] = 'D';
        else if (strcmp(temp, "1110") == 0) hex[HEX_DIGITS-i] = 'E';
        else if (strcmp(temp, "1111") == 0) hex[HEX_DIGITS-i] = 'F';
    }

    printf("\nHex Result:  %s\n", hex);
}

// INVERT ARRAY ======================================================================
/* flip 1's and 0's in a binary array (for subtraction)                             */
void invert_array(int* array, int size){

    for (int i = 0; i < size; i++){
        array[i] = !array[i];
    }
}

// BIT LEVEL PROPAGATE AND GENERATE ==================================================
/* Calculate propagate and generate functions at the bit level                      */
void bit_level_p_and_g(int* propagate, int* generate, int* A, int* B, int size){
    for (int i = 0; i < size; i++){
        propagate[i] = A[i] || B[i];
        generate[i] = A[i] && B[i];
    }
}

// NEXT LEVEL PROPAGATE AND GENERATE =================================================
/* Calculate propagate and generate functions at any level above the bit level      */
void next_level_P_and_G(int* propagate_next, int* generate_next, int* p, int* g, int size, int block){

    int p_temp;
    int g_temp;
    for (int j = 0; j < size/block; j++){
        p_temp = 1;
        g_temp = 0;

        // only true if the carry is propagated along the whole group
        for (int i = 0; i < 4; i++){
            p_temp *= p[j*4 + i];
        }

        // true if a carry is generated at the last bit or if a previously generated carry is propagated to the last bit in the group
        g_temp = g[4*j+3]   ||   p[4*j+3] * g[4*j+2]   ||   p[4*j+3] * p[4*j+2] * g[4*j+1]   ||   p[4*j+3] * p[4*j+2] * p[4*j+1] * g[4*j];

        propagate_next[j] = p_temp;
        generate_next[j] = g_temp;
    }
}

// TOP LEVEL CARRY ===================================================================
/* Calculate the carry at the top level (e.g. super section)                        */
void top_level_carry(int* p, int* g, int* carry_in, int size, int block){

    // Never carry in to the first bit
    carry_in[0] = 0;

    for (int i = 1; i < size/block; i++){
        carry_in[i] = g[i-1]  ||  (p[i-1] && carry_in[i-1]);
    }
}

// LOWER LEVEL CARRY =================================================================
/* Calculate the carry for any level that is not the top level                      */
void lower_level_carry(int* p, int* g, int* carry_in, int* group_carry, int size, int block){
    for (int j = 0; j < size/block; j++){
        carry_in[4*j+1] = g[4*j]  ||  (p[4*j] && group_carry[j]);

        for (int i = 1; i < 4; i++){
            carry_in[4*j+i+1] = g[4*j+i]  ||  (p[4*j+i] && carry_in[4*j+i]);
        }
    }
    // Never carry into the first bit
    carry_in[0] = 0;
}


// SUM ===============================================================================
/* Ripple add the two numbers and the calculated carries (would be parallel
    in hardware)                                                                    */
void sum(int* result, int* A, int* B, int* carry_in, int c_in, int size){
    for (int i = 0; i < size; i++)
        result[i] = A[i] ^ B[i] ^ carry_in[i];
}

