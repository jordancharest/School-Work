/* be-careful.c */

/* Be sure to allocate space for the '\0' character when
    treating data as a printable character string! */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main()
{
  char * x = "ABCD";  /* note that *x is read-only memory */

  char name[5] = "David";      /* "David\0" */
  printf( "hi %s\n", name );

  name[1] = 'u';
  printf( "hi %s\n", name );


  char * path = malloc( 50 );
  strcpy( path, "/cs/goldsd/s18/os" );
  printf( "path is %s\n", path );


  return EXIT_SUCCESS;
}
