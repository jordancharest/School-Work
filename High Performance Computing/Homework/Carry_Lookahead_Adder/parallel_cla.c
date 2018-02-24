#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define HEX_DIGITS 262144
#define BITS HEX_DIGITS*4
#define BLOCK_SIZE 4
FILE* input_file;
FILE* output_file;

// FUNCTION DECLARATIONS ===================================
void user_input(int* binary_array);
void add_bits(int* binary_array, int index, int dig1, int dig2, int dig3, int dig4);
void print_binary_array(int* array, int size);
void invert_array(int* array, int size);

void bit_level_p_and_g(int* propagate, int* generate, int* A, int* B, int size);
void next_level_P_and_G(int* propagate_next, int* generate_next, int* p, int* g, int size, int block);
void top_level_carry(int* p, int* g, int* carry_in, int size, int block, int* gpc_prev, int world_rank);
void lower_level_carry(int* p, int* g, int* carry_in, int* group_carry, int size, int block, int* gpc_prev, int world_rank);

void carry_lookahead_adder(int* num1, int* num2, int* A, int* B, int c_in, int elements_per_proc, int world_rank, int world_size);
void array_to_hex_string(int* array);
void sum(int* result, int* A, int* B, int* carry_in, int c_in, int size);
void ripple_adder(int* A, int* B, int c_in);

// MAIN ====================================================
int main(int argc, char** argv) {

    MPI_Init(NULL, NULL);
    //double start = MPI_Wtime();
    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int* A = (int*)malloc(BITS * sizeof(int));
    int* B = (int*)malloc(BITS * sizeof(int));

    if (world_rank == 0) {
	if ((input_file = fopen(argv[1], "r")) == NULL){
            perror ("ERROR: fopen() failed");
            exit(EXIT_FAILURE);
        }

        if ((output_file = fopen(argv[2], "w")) == NULL){
            perror ("ERROR: fopen() failed");
            exit(EXIT_FAILURE);
        }

        // Get two hex numbers to add, convert to binary arrays
    	user_input(A);
    	user_input(B);
    }

    // don't allow other ranks to attempt calculations until the numbers are input
    MPI_Barrier(MPI_COMM_WORLD);

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

#ifdef RIPPLE // performance study purposes
    ripple_adder(A,B,c_in);
#else

    // all arrays need to be dynamically allocated since they are larger than the stack
    int elements_per_proc = BITS/world_size;
    int* sub_A = malloc(sizeof(int) * elements_per_proc);
    int* sub_B = malloc(sizeof(int) * elements_per_proc);

    MPI_Scatter(A, elements_per_proc, MPI_INT, sub_A, elements_per_proc, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(B, elements_per_proc, MPI_INT, sub_B, elements_per_proc, MPI_INT, 0, MPI_COMM_WORLD);

    carry_lookahead_adder(A, B, sub_A, sub_B, c_in, elements_per_proc, world_rank, world_size);

    free(sub_A);
    free(sub_B);
    if (world_rank == 0) {
        free(A);
        free(B);
	fclose(input_file);
	fclose(output_file);
    }

#endif

#ifdef TIME
    printf("Time: %f\n", (MPI_Wtime() - start));
#endif


    MPI_Finalize();

    return EXIT_SUCCESS;
}

// USER INPUT ========================================================================
/* Prompt the user for (or scan from file) a hex string, then convert the string
    to an array of 1's and 0's (binary array)                                       */
void user_input(int* binary_array) {

    //printf("\n\nPlease enter a %d-digit (%d-bit) hexadecimal number:\n", HEX_DIGITS, BITS);
    char* hex = (char*)malloc(HEX_DIGITS * sizeof(char)+1);
    fscanf(input_file, "%s", hex);

    //printf("\n\n\nYou entered: %s\n", hex);
    //printf("The equivalent binary is:\n");

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
            //default:
                //printf("\n Invalid hex digit %c\n", hex[i]);
        }

        ++i;
    }
    //print_binary_array(binary_array, BITS);
    free(hex);
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
void carry_lookahead_adder(int* num1, int* num2, int* A, int* B, int c_in,
	                   int elements_per_proc, int world_rank, int world_size) {

    // Prepare to receive generates, propagates, and carries from previous processors
    int gpc_top[3] = {0};
    int gpc_super_section[3] = {0};
    int gpc_section[3] = {0};
    int gpc_group[3] = {0};
    int gpc_bit[3] = {0};
    MPI_Request request;
    MPI_Request request_top;
    MPI_Request request_super_section;
    MPI_Request request_section;
    MPI_Request request_group;
    MPI_Request request_bit;

    // world rank 0 does only sends, the last world rank only receives
    if (world_rank > 0) {
        MPI_Irecv(gpc_top, 3, MPI_INT, world_rank-1, 0, MPI_COMM_WORLD, &request_top);
        MPI_Irecv(gpc_super_section, 3, MPI_INT, world_rank-1, 0, MPI_COMM_WORLD, &request_super_section);
        MPI_Irecv(gpc_section, 3, MPI_INT, world_rank-1, 0, MPI_COMM_WORLD, &request_section);
        MPI_Irecv(gpc_group, 3, MPI_INT, world_rank-1, 0, MPI_COMM_WORLD, &request_group);
        MPI_Irecv(gpc_bit, 3, MPI_INT, world_rank-1, 0, MPI_COMM_WORLD, &request_bit);
    }

    int block = 1;
    int* propagate = malloc(sizeof(int) * elements_per_proc);
    int* generate = malloc(sizeof(int) * elements_per_proc);
    bit_level_p_and_g(propagate, generate, A, B, elements_per_proc);

#ifdef BARRIER
    MPI_Barrier(MPI_COMM_WORLD);
#endif

    // GROUP: Divide into groups, calculate propagate and generate functions for each group
    int* group_propagate = malloc(sizeof(int) * elements_per_proc / BLOCK_SIZE);
    int* group_generate = malloc(sizeof(int) * elements_per_proc / BLOCK_SIZE);
    block *= BLOCK_SIZE;
    next_level_P_and_G(group_propagate, group_generate, propagate, generate, elements_per_proc, block);

#ifdef BARRIER
    MPI_Barrier(MPI_COMM_WORLD);
#endif

    // SECTION
    int* section_propagate = malloc(sizeof(int) * elements_per_proc / pow(BLOCK_SIZE,2));
    int* section_generate = malloc(sizeof(int) * elements_per_proc / pow(BLOCK_SIZE,2));
    block *= BLOCK_SIZE;
    next_level_P_and_G(section_propagate, section_generate, group_propagate, group_generate, elements_per_proc, block);

#ifdef BARRIER
    MPI_Barrier(MPI_COMM_WORLD);
#endif

    // SUPER SECTION
    int* super_section_propagate = malloc(sizeof(int) * elements_per_proc / pow(BLOCK_SIZE,3));
    int* super_section_generate = malloc(sizeof(int) * elements_per_proc / pow(BLOCK_SIZE,3));
    block *= BLOCK_SIZE;
    next_level_P_and_G(super_section_propagate, super_section_generate, section_propagate, section_generate, elements_per_proc, block);

#ifdef BARRIER
    MPI_Barrier(MPI_COMM_WORLD);
#endif

    // SUPER SUPER SECTION
    int* super_super_section_propagate = malloc(sizeof(int) * elements_per_proc / pow(BLOCK_SIZE,4));
    int* super_super_section_generate = malloc(sizeof(int) * elements_per_proc / pow(BLOCK_SIZE,4));
    block *= BLOCK_SIZE;
    next_level_P_and_G(super_super_section_propagate, super_super_section_generate, super_section_propagate, super_section_generate, elements_per_proc, block);

#ifdef BARRIER
    MPI_Barrier(MPI_COMM_WORLD);
#endif

    // SUPER SUPER SECTION: head back down to bit level, calculating the carry-in along the way
    int* super_super_section_carry = malloc(sizeof(int) * elements_per_proc / pow(BLOCK_SIZE,4));
    if (world_rank > 0) MPI_Wait(&request_top, MPI_STATUS_IGNORE);
    top_level_carry(super_super_section_propagate, super_super_section_generate, super_super_section_carry, elements_per_proc, block, gpc_top, world_rank);

    // Prepare and send generate, propagate, and carry to next process
    int gen_prop_carry[3] = {0};
    if (world_rank != (world_size-1)) {
        gen_prop_carry[0] = super_super_section_generate[elements_per_proc/block - 1];
        gen_prop_carry[1] = super_super_section_propagate[elements_per_proc/block - 1];
        gen_prop_carry[2] = super_super_section_carry[elements_per_proc/block - 1];

        MPI_Isend(gen_prop_carry, 3, MPI_INT, world_rank+1, 0, MPI_COMM_WORLD,  &request);
    }

    // SUPER SECTION: head back down to bit level, calculating the carry-in along the way
    int* super_section_carry = malloc(sizeof(int) * elements_per_proc / pow(BLOCK_SIZE,3));
    if (world_rank > 0) MPI_Wait(&request_super_section, MPI_STATUS_IGNORE);
    lower_level_carry(super_section_propagate, super_section_generate, super_section_carry, super_super_section_carry, elements_per_proc, block, gpc_super_section, world_rank);
    block /= BLOCK_SIZE;

    // Prepare and send generate, propagate, and carry to next process
    if (world_rank != (world_size-1)) {
        gen_prop_carry[0] = super_section_generate[elements_per_proc/block - 1];
        gen_prop_carry[1] = super_section_propagate[elements_per_proc/block - 1];
        gen_prop_carry[2] = super_section_carry[elements_per_proc/block - 1];

        MPI_Isend(gen_prop_carry, 3, MPI_INT, world_rank+1, 0, MPI_COMM_WORLD,  &request);
    }

    // SECTION
    int* section_carry = malloc(sizeof(int) * elements_per_proc / pow(BLOCK_SIZE,2));
    if (world_rank > 0) MPI_Wait(&request_section, MPI_STATUS_IGNORE);
    lower_level_carry(section_propagate, section_generate, section_carry, super_section_carry, elements_per_proc, block, gpc_section, world_rank);
    block /= BLOCK_SIZE;

    // Prepare and send generate, propagate, and carry to next process
    if (world_rank != (world_size-1)) {
        gen_prop_carry[0] = section_generate[elements_per_proc/block - 1];
        gen_prop_carry[1] = section_propagate[elements_per_proc/block - 1];
        gen_prop_carry[2] = section_carry[elements_per_proc/block - 1];

        MPI_Isend(gen_prop_carry, 3, MPI_INT, world_rank+1, 0, MPI_COMM_WORLD,  &request);
    }


    // GROUP
    int* group_carry = malloc(sizeof(int) * elements_per_proc / BLOCK_SIZE);
    if (world_rank > 0) MPI_Wait(&request_group, MPI_STATUS_IGNORE);
    lower_level_carry(group_propagate, group_generate, group_carry, section_carry, elements_per_proc, block, gpc_group, world_rank);
    block /= BLOCK_SIZE;

    // Prepare and send generate, propagate, and carry to next process
    if (world_rank != (world_size-1)) {
        gen_prop_carry[0] = group_generate[elements_per_proc/block - 1];
        gen_prop_carry[1] = group_propagate[elements_per_proc/block - 1];
        gen_prop_carry[2] = group_carry[elements_per_proc/block - 1];

        MPI_Isend(gen_prop_carry, 3, MPI_INT, world_rank+1, 0, MPI_COMM_WORLD,  &request);
    }


    // BIT LEVEL
    int* carry_in = malloc(sizeof(int) * elements_per_proc);

    if (world_rank > 0) MPI_Wait(&request_bit, MPI_STATUS_IGNORE);
    lower_level_carry(propagate, generate, carry_in, group_carry, elements_per_proc, block, gpc_bit, world_rank);

    // Prepare and send generate, propagate, and carry to next process
    if (world_rank != (world_size-1)) {
        gen_prop_carry[0] = generate[elements_per_proc - 1];
        gen_prop_carry[1] = propagate[elements_per_proc - 1];
        gen_prop_carry[2] = carry_in[elements_per_proc - 1];

        MPI_Isend(gen_prop_carry, 3, MPI_INT, world_rank+1, 0, MPI_COMM_WORLD,  &request);
    }



    // CALCULATE RESULT using bit level propagate, generate, and carry
    int* total_carry = malloc(BITS * sizeof(int));

    MPI_Gather(carry_in, elements_per_proc, MPI_INT, total_carry, elements_per_proc, MPI_INT, 0, MPI_COMM_WORLD);

    int* result = malloc(BITS * sizeof(int));
    if (world_rank == 0) {
        sum(result, num1, num2, total_carry, c_in, BITS);
        array_to_hex_string(result);
    }



#ifdef DEBUG_MODE
    int message;
    if (world_rank == 1)
	MPI_Recv(&message, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);


    if (world_rank == 0) {
        printf("\n\nBIT Level");
        printf("\nPropagate: ");
        print_binary_array(propagate, elements_per_proc);
        printf("\nGenerate:  ");
        print_binary_array(generate, elements_per_proc);
        printf("\nCarry:     ");
        print_binary_array(carry_in, elements_per_proc);

        printf("\n\nGROUP Level");
        printf("\nPropagate: ");
        print_binary_array(group_propagate, elements_per_proc/4);
        printf("\nGenerate:  ");
        print_binary_array(group_generate, elements_per_proc/4);
        printf("\nCarry:     ");
        print_binary_array(group_carry, elements_per_proc/4);

        printf("\n\nSECTION Level");
        printf("\nPropagate: ");
        print_binary_array(section_propagate, elements_per_proc/16);
        printf("\nGenerate:  ");
        print_binary_array(section_generate, elements_per_proc/16);
        printf("\nCarry:     ");
        print_binary_array(section_carry, elements_per_proc/16);

        printf("\n\nSUPER SECTION Level");
        printf("\nPropagate: ");
        print_binary_array(super_section_propagate, elements_per_proc/64);
        printf("\nGenerate:  ");
        print_binary_array(super_section_generate, elements_per_proc/64);
        printf("\nCarry:     ");
        print_binary_array(super_section_carry, elements_per_proc/64);


        printf("\n\nSUM\n");
        print_binary_array(A, elements_per_proc);
        printf("\n");
        print_binary_array(B, elements_per_proc);
        printf("\n");
        print_binary_array(carry_in, elements_per_proc);
        printf("\n");
        print_binary_array(result, elements_per_proc);
        printf("\n\n");
    }


    if (world_rank == 0) {
        message = 1;
        MPI_Send(&message, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
    }
#endif

    free(propagate);
    free(generate);
    free(group_propagate);
    free(group_generate);
    free(section_propagate);
    free(section_generate);
    free(super_section_propagate);
    free(super_section_generate);
    free(total_carry);
    free(result);
}

// ARRAY TO HEX STRING ===================================================================
/* Converts a binary array back to a hex string for output                              */
void array_to_hex_string(int* array){

    char* hex = malloc(HEX_DIGITS+1);
    hex[HEX_DIGITS] = '\0';

    char temp[5];
    temp[4] = '\0';

    for(int i = HEX_DIGITS; i > 0; i--){
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

    fprintf(output_file,"%s\n", hex);
    fflush(stdout);
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
void top_level_carry(int* p, int* g, int* carry_in, int size, int block,
                     int* gpc_prev, int world_rank) {

    // Never carry in to the first bit
    if (world_rank == 0)
        carry_in[0] = 0;
    else  // determine the carry from the previous process
        carry_in[0] = gpc_prev[0] || (gpc_prev[1] && gpc_prev[2]);

    for (int i = 1; i < size/block; i++){
        carry_in[i] = g[i-1]  ||  (p[i-1] && carry_in[i-1]);
    }
}

// LOWER LEVEL CARRY =================================================================
/* Calculate the carry for any level that is not the top level                      */
void lower_level_carry(int* p, int* g, int* carry_in, int* group_carry, int size,
                       int block, int* gpc_prev, int world_rank) {

    // Never carry in to the first bit
    if (world_rank == 0)
        carry_in[0] = 0;
    else  // determine the carry from the previous process
        carry_in[0] = gpc_prev[0] || (gpc_prev[1] && gpc_prev[2]);

    for (int j = 0; j < size/block; j++){
        carry_in[4*j+1] = g[4*j]  ||  (p[4*j] && group_carry[j]);

        for (int i = 1; i < 4; i++){
            carry_in[4*j+i+1] = g[4*j+i]  ||  (p[4*j+i] && carry_in[4*j+i]);
        }
    }
}


// SUM ===============================================================================
/* Ripple add the two numbers and the calculated carries (would be parallel
    in hardware)                                                                    */
void sum(int* result, int* A, int* B, int* carry_in, int c_in, int size){
    for (int i = 0; i < size; i++)
        result[i] = A[i] ^ B[i] ^ carry_in[i];
}


// RIPPLE ADDER ======================================================================
/* Calculate the carry as you go for performance study purposes (no potential for
    parallelization)                                                                */
void ripple_adder(int* A, int* B, int c_in) {

    int carry = c_in;
    int* result = (int*)malloc(BITS * sizeof(int));
    for (int i = 0; i < BITS; i++) {
        result[i] = A[i] ^ B[i] ^ carry;

        // calculate carry
        if ((A[i] && B[i]) || (A[i] && carry) || (B[i] && carry))
            carry = 1;
        else
            carry = 0;

    }

    array_to_hex_string(result);
}
