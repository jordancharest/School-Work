/* fd-open-redirect.c */

/* also check out dup2() */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#if 0

  file descriptor (fd)

  -- each process has a file descriptor table associated with it
      that keeps track of its open files (a.k.a., byte streams)

  fd         C++     Java           C
  0 stdin    cin     System.in      scanf(), read(), getchar(), ...
  1 stdout   cout    System.out     printf(), write(), putchar(), ...
  2 stderr   cerr    System.err     fprintf( stderr, "ERROR: ....\n" );
                                    perror( "open() failed" );

  stdout and stderr (by default) are both displayed on the terminal

  stdout is buffered output
   (therefore, we use '\n' at the end of printf() statements and
     also fflush() to explicitly flush the output buffer)

  stderr (fd 2) is not buffered

#endif

int main()
{
  char name[80];
  strcpy( name, "testfile.txt" );

  close( 1 );  /* close stdout */

   /* fd table:    fd
                   0  stdin
                   1
                   2  stderr */

#if 0
  printf( "Where does this line of output go?\n" );
  return EXIT_SUCCESS;
#endif

                    /* attempt to open this file:
                        O_WRONLY for writing
                        O_CREAT create the file if necessary
                        O_TRUNC truncate the file if it already exists,
                                 which erases the file contents
                        (without O_TRUNC, (over)writing starts at byte 0
                        (check out O_APPEND in the man page) */
  int fd = open( name, O_WRONLY | O_CREAT | O_TRUNC, 0660 );
                                                 /*  ^^^^
                                                      |
                                       leading 0 means this is an octal number

                                      0660 ==> 110 110 000
                                               rwx rwx rwx
                                               ^^^ ^^^ ^^^
                                                |   |   |
                                                |   |  other/world permissions
                                                |   |
                                                |  group permissions
                                                |
                                             rw for file owner */

  if ( fd == -1 )
  {
    perror( "open() failed" );
    return EXIT_FAILURE;
  }

  /*    fd table:
        0 stdin
        1 testfile.txt [O_WRONLY]
        2 stderr
  */

  printf( "fd is %d\n", fd );   /* the '\n' character here will flush the
                                    stdout buffer if we're printing this to
                                     the terminal

                                   if we're redirecting to a file, then the
                                    '\n' will not flush the buffer */
  fflush( stdout );

  char buffer[80];
  sprintf( buffer, "ABCDEFGHIJKLMNOPQRSTUVWXYZ" );

  int rc = write( fd, buffer, 18 );
   /* to do: check for error here.... */

  printf( "wrote %d bytes to the file\n", rc );
/*  fflush( stdout ); */

  close( fd );   /* remove entry 1 from the fd table */
    /* the close() does not automatically flush the stdout buffer here... */

  return EXIT_SUCCESS;
}

