/* buffering.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
  int x = 42;   /* x is statically allocated memory
                   (4 bytes are allocated on the stack) */

  printf( "sizeof int is %d\n", (int)sizeof( int ) );
  printf( "x is %d", x );
   /* add a newline '\n' above to make sure that the buffered output
      is actually displayed to the terminal (i.e., flushed) */


  int * y = NULL;   /* y is statically allocated memory
                       that will (hopefully) point to a
                       valid memory addr */

  printf( "sizeof int* is %d", (int)sizeof( int * ) );
   /* add a newline '\n' above to make sure that the buffered output
      is actually displayed to the terminal (i.e., flushed) */


  /* use fflush( stdout ) here to explicitly flush the output buffer
      to the terminal */
  fflush( stdout );

  printf( "y is %d\n", *y );

  return EXIT_SUCCESS;
}
