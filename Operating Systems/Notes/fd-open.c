/* fd-open.c */

/**

  file descriptor (fd)

  -- each process has a file descriptor table associated with it
      that keeps track of its open files (i.e., byte streams)

  fd         C++    Java         C
  0 stdin    cin    System.in    scanf(), read(), getchar(), ...
  1 stdout   cout   System.out   printf(), write(), putchar(), ...
  2 stderr   cerr   System.err   fprintf( stderr, "ERROR: ....\n" );
                                 perror( "open() failed" );

  stdout and stderr (by default) are both displayed on the terminal

  stdout is buffered output
   (therefore, we rely on '\n' in printf() to flush the buffer or
     we can use fflush( stdout ) or fflush( NULL )

  stderr (fd 2) is never buffered

**/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main()
{
  char * c;    /* c is a pointer */
  c = malloc( 80 );
  char d[80];  /* d is a chunk of 80 bytes */
  c[5] = d[5] = 12;


  char * name = "testfile.txt";

  printf( "file name is %s\n", name );

#if 0
  name[5] = 'q';   /* this will seg-fault because "testfile.txt" is
                       stored in the code segment, which is read-only mem */
  printf( "file name is %s\n", name );
#endif


  int fd = open( name, O_RDONLY );

  if ( fd == -1 )
  {
    perror( "open() failed" );
    return EXIT_FAILURE;
  }

  printf( "fd is %d\n", fd );

  /** fd table:
      0 stdin
      1 stdout
      2 stderr
      3 testfile.txt [O_RDONLY]
  **/


  char buffer[80];

int j = 5;
while ( j > 0 )
{
  int rc = read( fd, buffer, 20 );

  if ( rc == -1 )
  {
    perror( "read() failed" );
    return EXIT_FAILURE;
  }

  /* assume the data is printable string data */
  buffer[rc] = '\0';

  printf( "rc is %d\n", rc );
  printf( "READ: [%s]\n", buffer );

  j--;
}

  close( fd );



  return EXIT_SUCCESS;
}
