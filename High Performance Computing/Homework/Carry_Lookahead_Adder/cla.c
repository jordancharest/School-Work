#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define HEX_DIGITS 256
#define BITS HEX_DIGITS*4


// FUNCTION DECLARATIONS ===================================
void int_to_array(int* array, long long num);
void array_to_int(int* array, long long* num);
void invert_array(int* array, int size);
void bit_level_p_and_g(int* propagate, int* generate, int* A, int* B, int size);
void next_level_P_and_G(int* propagate_next, int* generate_next, int* p, int* g, int size, int block);
void top_level_carry(int* propagate16, int* generate16, int* carry_in16, int size, int block);
void lower_level_carry(int* p, int* g, int* carry_in, int* group_carry, int size, int block);
void sum(int* result, int* A, int* B, int* carry_in, int c_in, int size);
void print_binary_array(int* array, int size);

void carry_lookahead_adder(int* A, int* B, int c_in);
void add_bits(int* binary_array, int index, int dig1, int dig2, int dig3, int dig4);
void user_input(int* binary_array);


//void print_binary(long long number, int integers);
//long long invert(long long num);


// MAIN ====================================================
int main()
{
    /*
    long long a, b;
    int A[sizeof(a)*8] = {0};    // binary array representation of a
    int B[sizeof(a)*8] = {0};    // binary array representation of b


    printf("Enter A (hex):\n");
    scanf("%llx", &a);
    printf("Enter B (hex):\n");
    scanf("%llx", &b);

    int_to_array(A, a);
    int_to_array(B, b);
    */







    int A[BITS] = {0};
    user_input(A);
    int B[BITS] = {0};
    user_input(B);

    int operation;
    printf("\nAdd (0) or subtract (1):\n");
    scanf("%d", &operation);

    /*
    printf("\nA is %016llx or %lld", a, a);
    printf("\nB is %016llx or %lld", b, b);
    */

    int c_in = 0;
    if (operation){      // subtraction
        c_in = 1;
        printf("\nInverting...");
        invert_array(B, BITS);
        printf("\nB (bin) : ");
        print_binary_array(B, BITS);
    }




    printf("\n\nCalculate sum, S:\n");

    carry_lookahead_adder(A, B, c_in);


    return 0;
}

// USER INPUT =====================================================================
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
                printf("\n Invalid hex digit %c ", hex[i]);
        }

        ++i;
    }

    print_binary_array(binary_array, BITS);
}


// ADD BITS ==========================================================================
void add_bits(int* binary_array, int index, int dig1, int dig2, int dig3, int dig4) {

    binary_array[BITS - index*4 - 1] = dig1;
    binary_array[BITS - index*4 - 2] = dig2;
    binary_array[BITS - index*4 - 3] = dig3;
    binary_array[BITS - index*4 - 4] = dig4;
}

// PRINT BINARY ARRAY ============================================================
void print_binary_array(int* array, int size){

    for (int i = size-1; i >= 0; i--){
        printf("%d", array[i]);
    }
}



// CARRY LOOKAHEAD ADDER ==========================================================
void carry_lookahead_adder(int* A, int* B, int c_in){
// Divide into 4 groups, calculate propagate and generate for each group
    int block = 1;
    int propagate[BITS] = {0};
    int generate[BITS] = {0};
    bit_level_p_and_g(propagate, generate, A, B, BITS);


    // need to be changed if block size changes
    int propagate4[BITS/4] = {0};
    int generate4[BITS/4] = {0};

    // Divide into 4 groups again, calculate propagate and generate for each group
    block *= 4;
    next_level_P_and_G(propagate4, generate4, propagate, generate, BITS, block);


    // need to be changed if block size changes
    int propagate16[BITS/16] = {0};
    int generate16[BITS/16] = {0};

    // Again
    block *= 4;
    next_level_P_and_G(propagate16, generate16, propagate4, generate4, BITS, block);



    // Now head back down to bit level, calculating the carry-in along the way

    int carry_in16[BITS/16] = {0};      // change if block size changes; should be same size as propagate16
    top_level_carry(propagate16, generate16, carry_in16, BITS, block);

    int carry_in4[BITS/4] = {0};      // change if block size changes; should be same size as propagate4
    lower_level_carry(propagate4, generate4, carry_in4, carry_in16, BITS, block);


    block /= 4;
    int carry_in[BITS] = {0};

    lower_level_carry(propagate, generate, carry_in, carry_in4, BITS, block);



    int result[BITS] = {0};
    sum(result, A, B, carry_in, c_in, BITS);

    long long int_result = 0;
    array_to_int(result, &int_result);





    printf("\nA (bin) : ");
    print_binary_array(A, BITS);
    printf("\nB (bin) : ");
    print_binary_array(B, BITS);

    /*
    printf("\nPropagate  : ");
    print_binary_array(propagate, sizeof(a)*8);
    printf("\nGenerate   : ");
    print_binary_array(generate, sizeof(a)*8);


    printf("\nPropagate4 : ");
    print_binary_array(propagate4, sizeof(a)*2);
    printf("\nGenerate4  : ");
    print_binary_array(generate4, sizeof(a)*2);


    printf("\nPropagate16: ");
    print_binary_array(propagate16, sizeof(a)/2);
    printf("\nGenerate16 : ");
    print_binary_array(generate16, sizeof(a)/2);


    printf("\nCarry16    : ");
    print_binary_array(carry_in16, sizeof(a)/2);

    printf("\nCarry4     : ");
    print_binary_array(carry_in4, sizeof(a)*2);

    printf("\nCarry      : ");
    print_binary_array(carry_in, sizeof(a)*8);
    */

    printf("\nS (bin) : ");
    print_binary_array(result, BITS);

    printf("\n\nS is %016llx or %lld", int_result, int_result);

}


// INT TO ARRAY ===================================================================
void int_to_array(int* array, long long number){

    unsigned int NO_OF_BITS = sizeof(number)*8;

    for (int i = 0; i < NO_OF_BITS; i++){
        array[i] = number & 1;
        number >>= 1;
    }
}

// ARRAY TO INT ==================================================================
void array_to_int(int* array, long long* number){

    unsigned int NO_OF_BITS = sizeof(*number)*8;

    for (int i = NO_OF_BITS - 1; i >= 0; i--){
        *number <<= 1;

        if (array[i])
            *number |= 1;
    }
}

// INVERT ARRAY ==================================================================
void invert_array(int* array, int size){

    for (int i = 0; i < size; i++){
        array[i] = !array[i];
    }
}

// BIT LEVEL PROPAGATE AND GENERATE ==============================================
void bit_level_p_and_g(int* propagate, int* generate, int* A, int* B, int size){

    for (int i = 0; i < size; i++){
        propagate[i] = A[i] || B[i];
        generate[i] = A[i] && B[i];
    }
}

// NEXT LEVEL PROPAGATE AND GENERATE ==============================================
void next_level_P_and_G(int* propagate_next, int* generate_next, int* p, int* g, int size, int block){

    int p_temp;
    int g_temp;
    for (int j = 0; j < size/block; j++){
        p_temp = 1;
        g_temp = 0;

        for (int i = 0; i < 4; i++){
            p_temp *= p[j*4 + i];
        }

        g_temp = g[4*j+3]  +  p[4*j+3] * g[4*j+2]  +  p[4*j+3] * p[4*j+2] * g[4*j+1]  +  p[4*j+3]*p[4*j+2]*p[4*j+1]*g[4*j];

        propagate_next[j] = p_temp;
        generate_next[j] = g_temp;
    }
}

// TOP LEVEL CARRY ===============================================================
void top_level_carry(int* p, int* g, int* carry_in, int size, int block){

    carry_in[0] = g[0];

    for (int i = 1; i < size/block; i++){
        carry_in[i] = g[i]  ||  (p[i] && carry_in[i-1]);
    }

}

// LOWER LEVEL CARRY =============================================================
void lower_level_carry(int* p, int* g, int* carry_in, int* group_carry, int size, int block){

    //printf("\n\nDEBUGGING\n\n");
    //printf("Group carry: %d\n", size/block);
    //printf("g + p*c = c_in");

    for (int j = 0; j < size/block; j++){
        carry_in[4*j] = g[4*j]  ||  (p[4*j] && group_carry[j]);
        //printf("\n%d + %d*%d = %d\n", g[4*j], p[4*j], group_carry[j], carry_in[4*j]);

        for (int i = 1; i < 4; i++){
            carry_in[4*j+i] = g[4*j+i]  ||  (p[4*j+i] && carry_in[4*j+i-1]);
            //printf("%d + %d*%d = %d\n", g[4*j+i], p[4*j+i], carry_in[4*j+i-1], carry_in[4*j+i]);
        }
    }
}


// SUM ===========================================================================
void sum(int* result, int* A, int* B, int* carry_in, int c_in, int size){

    result[0] = (A[0] ^ B[0]) ^ c_in;
    for (int i = 1; i < size; i++)
        result[i] = (A[i] ^ B[i]) ^ carry_in[i-1];

}

/*
// PRINT BINARY ==================================================================
void print_binary(long long number, int integers){

    // pop off the least significant digit and call print_binary again
    if (number) {
        integers++;                             // track the number of integers
        print_binary(number >> 1, integers);
        putc((number & 1) ? '1' : '0', stdout);

    // when there are no more digits to pop off, print leading zeros
    } else if (integers < 64){
        putc('0', stdout);
        integers++;
        print_binary(0, integers);
    }
}


// INVERT ========================================================================
long long invert(long long num){

    unsigned int  NO_OF_BITS = sizeof(num) * 8;
    long long inverted_num = 0;
    for (int i = 0; i < NO_OF_BITS; i++){
        if((num & (1 << i)))
           inverted_num |= 1 << ((NO_OF_BITS - 1) - i);
    }
    return inverted_num;
}
*/
