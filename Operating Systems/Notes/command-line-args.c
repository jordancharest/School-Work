/* command-line-args.c */

#include <stdio.h>
#include <stdlib.h>

int main( int argc, char * argv[] )
{
  printf( "argc is %d\n", argc );    /* argument count */

  if ( argc != 2 )
  {
    fprintf( stderr, "ERROR: Invalid arguments\n" );
    fprintf( stderr, "USAGE: %s <filename>\n", argv[0] );
    return EXIT_FAILURE;
  }

  argv[0][2] = 'A';

  printf( "argv[0] is %s\n", argv[0] );
  printf( "argv[1] is %s\n", argv[1] );
#if 0
  printf( "argv[2] is %s\n", argv[2] );
  printf( "argv[3] is %s\n", argv[3] );
#endif


#if 0

int main( int argc, char * argv[] )

  bash$ ./a.out abcd efgh ijkl

   argv
   +--+     +--------------+
[0]| ======>| "./a.out\0"  |
   |  |     +--------------+
   +--+                        +---------+
[1]| =========================>| "abcd"  |
   |  |                        +---------+
   +--+
[2]| =====>
   |  |
   +--+
[3]| =======>
   |  |
   +--+
   etc.


#endif


  return EXIT_SUCCESS;
}
