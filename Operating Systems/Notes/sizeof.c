/* sizeof.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
  int y = 12;
  /* x points to dynamically allocated memory (4 bytes) */
  int * x = malloc( sizeof( int ) );
  *x = y; 

  free( x );  x = NULL;

  /* x now points to dynamically allocated memory
      (i.e., an array of 12 integers) */
  x = calloc( 12, sizeof( int ) );

  printf( "sizeof( x ) is %d\n", (int)sizeof( x ) );
  printf( "sizeof( *x ) is %d\n", (int)sizeof( *x ) );

  return EXIT_SUCCESS;
}
