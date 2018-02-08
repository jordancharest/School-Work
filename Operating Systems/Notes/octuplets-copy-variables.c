/* octuplets-copy-variables.c */

/** based on a true story... **/

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

/* function prototypes */
int child( char * data, int j, int k );
void parent( int children );

int main()
{
  /* create all the child processes */

  int i, children = 8;  /* based on a true story.... */

  int j = 0, k = 3;

  char data[1024];  /*   j     k
                         V     V  */
  sprintf( data, "ABCDEFGHIJKLMNOPQRSTUVWXYZZZZZZZZZZZZZZZZ" );

  for ( i = 0 ; i < children ; i++ )
  {
    pid_t pid = fork();    /* remember that the state of the parent process
                               is inherited by the child process here, so
                                each child has their own specific j and k values */
    if ( pid < 0 )
    {
      perror( "fork() failed" );
      return EXIT_FAILURE;
    }

    if ( pid == 0 )
    {
      int rc = child( data, j, k );
      exit( rc );   /* return rc; */
    }

#ifdef NO_PARALLEL
    wait( NULL );
#endif

    j += 4;
    k += 4;
  }

#if 1
  parent( children );
#endif

  return EXIT_SUCCESS;
}



/* each child process sleeps for t seconds and returns t
  when it wakes up */
int child( char * data, int j, int k )
{
  printf( "CHILD: I'm new. My pid is %d.\n", getpid() );

  data[k+1] = '\0';
  printf( "CHILD: Working on %s\n", data + j );

  printf( "CHILD: I'm gonna nap for %d seconds.\n", j );
  sleep( j );
  return j;
}


void parent( int children )
{
#ifndef NO_PARALLEL
  printf( "PARENT: I'm waiting for my children to wake up.\n" );

  int status;  /* return status from each child process */
  pid_t child_pid;

  while ( children > 0 )
  {
    /* wait until a child process exits */
    child_pid = wait( &status );   /* BLOCK */

    children--;

    printf( "PARENT: child %d terminated...", child_pid );
    if ( WIFSIGNALED( status ) )
    {
      printf( "abnormally\n" );  /* core dump or kill or kill -9 */
    }
    else if ( WIFEXITED( status ) )
    {
      int rc = WEXITSTATUS( status );
      printf( "successfully with exit status %d\n", rc );
    }

    printf( "PARENT: %d children to go....\n", children );
  }
#endif

  printf( "PARENT: All of my children are awake!\n" );

}
