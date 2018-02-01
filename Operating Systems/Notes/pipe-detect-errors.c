/* pipe-detect-errors.c */

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

  pid_t pid_rc = fork();   /* fork() will copy the fd table to the child */

  if ( pid_rc == -1 )
  {
    perror( "fork() failed" );
    return EXIT_FAILURE;
  }

  /* fd tables:    [each process has its own fd table]

     THIS PICTURE IS WHAT BOTH PROCESSES LOOK LIKE AFTER THEY
      BOTH CALL close() BELOW....

     [PARENT]                                               [CHILD]
     0: stdin                                               0: stdin
     1: stdout                                              1: stdout
     2: stderr                     +--------+               2: stderr
     3: p[0] <========READ======== | buffer |               3: 
     4:                            | buffer | <===WRITE==== 4: p[1]
                                   +--------+
  */


  if ( pid_rc == 0 )  /* CHILD */
  {
    close( p[0] );   /* close the read end of the pipe */
    p[0] = -1;

    char * xyz = NULL;
    *xyz = '?';   /* segmentation fault... */

    /* write data to the pipe */
    int bytes_written = write( p[1], "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26 );
    printf( "CHILD: Wrote %d bytes to the pipe\n", bytes_written );
      /* to do: check the return value of write() for error... */
  }
  else /* pid_rc > 0     PARENT */
  {
    close( p[1] );   /* close the write end of the pipe */
    p[1] = -1;

    /* read from the pipe */
    char buffer[80];
    int bytes_read = read( p[0], buffer, 10 );   /* BLOCKING */

    if ( bytes_read == -1 )
    {
      perror( "read() failed" );
      return EXIT_FAILURE;
    }
    else if ( bytes_read == 0 )
    {
      printf( "PARENT: there are no write descriptors on the pipe\n" );
      printf( "PARENT: child process must have terminated...\n" );
      return EXIT_FAILURE;
    }

    buffer[bytes_read] = '\0';   /* assume the data is string data */
    printf( "PARENT: Read %d bytes: %s\n", bytes_read, buffer );
  }

  /* fd tables:    [each process has its own fd table]

     After the child process segfaults, no write descriptors on the pipe
      buffer, so read() will return 0 from that point forward

     [PARENT]                                               [CHILD]
     0: stdin                                               ZOMBIE
     1: stdout
     2: stderr                     +--------+
     3: p[0] <========READ======== | buffer |
     4:                            | buffer |
                                   +--------+
  */



  return EXIT_SUCCESS;
}
