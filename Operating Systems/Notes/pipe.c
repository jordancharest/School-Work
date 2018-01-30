/* pipe.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
  int p[2];    /* array to hold the two pipe (file) descriptors:
                  p[0] is the read end of the pipe
                  p[1] is the write end of the pipe */

  int rc = pipe( p );   /* the input argument p will be filled in
                           with the specific descriptors */
  if ( rc == -1 )
  {
    perror( "pipe() failed" );
    return EXIT_FAILURE;
  }


  /* fd table:    [each process has its own fd table]

     0: stdin
     1: stdout
     2: stderr                     +--------+
     3: p[0] <========READ======== | buffer | think of this buffer as
     4: p[1] =========WRITE======> | buffer |  a temporary hidden transient file...
                                   +--------+
  */

  printf( "Create pipe; p[0] is %d and p[1] is %d\n", p[0], p[1] );

  /* write data to the pipe */
  int bytes_written = write( p[1], "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26 );
  printf( "Wrote %d bytes to the pipe\n", bytes_written );
    /* to do: check the return value of write() for error... */

  /* read from the pipe */
  char buffer[80];
  int bytes_read = read( p[0], buffer, 10 );
  buffer[bytes_read] = '\0';   /* assume the data is string data */
  printf( "Read %d bytes: %s\n", bytes_read, buffer );

  /* read more data from the pipe */
  bytes_read = read( p[0], buffer, 10 );
  buffer[bytes_read] = '\0';   /* assume the data is string data */
  printf( "Read %d bytes: %s\n", bytes_read, buffer );

  /* read more data from the pipe */
  bytes_read = read( p[0], buffer, 10 );
  buffer[bytes_read] = '\0';   /* assume the data is string data */
  printf( "Read %d bytes: %s\n", bytes_read, buffer );


#if 0
/*** this fourth attempt to read() will block because there is no data
      on the pipe at this point ***/
  /* read more data from the pipe */
  bytes_read = read( p[0], buffer, 10 );
  buffer[bytes_read] = '\0';   /* assume the data is string data */
  printf( "Read %d bytes: %s\n", bytes_read, buffer );
#endif



  return EXIT_SUCCESS;
}
