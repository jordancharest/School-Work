/* fork-with-variables.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
  int x = 5;
  int * y = malloc( sizeof( int ) );

   /* variables x and y (and anything allocated on the stack or the heap)
       will be copied to the new child process via fork() */

  pid_t pid_rc;   /* process id (pid_t is unsigned short) */

  /* create a new (child) process */
  pid_rc = fork();

  /* fork() will (attempt to) create a new process by copying this
      existing/running process */

  if ( pid_rc == -1 )
  {
    perror( "fork() failed" );
    return EXIT_FAILURE;
  }

  if ( pid_rc == 0 )  /* CHILD PROCESS */
  {
    printf( "CHILD: happy birthday\n" );
    printf( "CHILD: my pid is %d\n", getpid() );
    x += 10;
    *y = 12;
    printf( "CHILD: x is now %d\n", x );
    printf( "CHILD: *y is now %d\n", *y );
  }
  else if ( pid_rc > 0 )  /* PARENT PROCESS */
  {
    printf( "PARENT: my child process has pid %d\n", pid_rc );
    printf( "PARENT: my pid is %d\n", getpid() );
    x = 567;
    *y = 4000;
    printf( "PARENT: x is now %d\n", x );
    printf( "PARENT: *y is now %d\n", *y );
  }

  /* both parent and child processes also run this code below: */

  printf( "Calling free() to free up memory allocated for y...\n" );
  free( y );

  return EXIT_SUCCESS;
}
