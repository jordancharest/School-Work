/* malloc.c */

/* compile this code from bash shell (or terminal):

   bash$ gcc -Wall -Werror main.c

   once compiled, execute this code as follows:

   bash$ ./a.out
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
  int x = 42;   /* x is statically allocated memory
                   (4 bytes are allocated on the stack) */

  printf( "sizeof int is %d\n", (int)sizeof( int ) );

  printf( "x is %d\n", x );


  int * y = NULL;   /* y is statically allocated memory
                       that will (hopefully) point to a
                       valid memory addr */

  printf( "sizeof int* is %d\n", (int)sizeof( int * ) );
  printf( "sizeof char* is %d\n", (int)sizeof( char * ) );
  printf( "sizeof void* is %d\n", (int)sizeof( void * ) );

  y = malloc( sizeof( int ) );  /* y points to 4 bytes of dynamically
                                    allocated memory on the runtime heap */


  if ( y == NULL )
  {
    fprintf( stderr, "ERROR: malloc() failed" );
    return EXIT_FAILURE;
  }

  *y = 456;

  printf( "*y is %d\n", *y );

  free( y );
  y = NULL;

#if 0
  printf( "*y is %d\n", *y );
#endif
  

  /* next, dynamically allocate memory for an array of size 100 */
  int * z = NULL;

#if 0
  z = malloc( 100 * sizeof( int ) );
#endif

  z = calloc( 100, sizeof( int ) );

  if ( z == NULL )
  {
    fprintf( stderr, "calloc() failed" );
    return EXIT_FAILURE;
  }

  z[10] = 123;
  *(z + 10) = 456;  /* ==> z + 10 * sizeof( int ) */

  printf( "z[10] is %d\n", z[10] );


  /* don't do this:   (this does NOT resize the array) */
  z[20000] = 555;
  printf( "z[20000] is %d\n", z[20000] );
    /* wow, this still works without crashing! */



  /* reallocate space (on the runtime heap) for z */
  /* (preserve the contents of the original memory pointed to by z) */
  z = realloc( z, 200 * sizeof( int ) );

  z[150] = 678;
  printf( "z[150] is %d\n", z[150] );

  free( z );





  return EXIT_SUCCESS;  /* 0 */   /* or EXIT_FAILURE */

   /* use "echo $?" in the shell to see the exit status
       of the last-terminated process */
}
