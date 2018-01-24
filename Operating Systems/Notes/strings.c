/* strings.c */

/* check out man pages for: strlen(), strcpy(), strncpy(),
          sprintf(), strcmp(), strncmp(), strstr(), strchr(),
          strcat(), strtok(), etc. */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main()
{
  char name[80];                   /* char == byte */
  strcpy( name, "Goldschmidt" );

  /*       +------------------------------------------+   */
  /* name: | Goldschmidt\0                            |   */
  /*       +------------------------------------------+   */

  /* '\0' is the end-of-string (EOS) character */

  name[5] = '\0';

  printf( "hello, %s\n", name );


  char x[10];

  x[0] = 'A';
  x[1] = 'B';

#if 0
  x[2] = '\0';
#endif

  x[3] = 'D';

  printf( "hello, %s !!!!!\n", x );

  /* undefined behavior here!  try on different platforms and
      you will see different results (because x[2] is uninitialized) */



  /* example of string concatenation */
  char path[100];
  int q = sprintf( path, "/cs/goldsd/" );
  strcpy( path + q, "ABC.txt" );

  printf( "path is %s\n", path );
  path[10] = '\0';
  printf( "path is %s\n", path );
  printf( "path + q is %s\n", path + q );

  return EXIT_SUCCESS;
}
