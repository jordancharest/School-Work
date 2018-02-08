/* octuplets.c */

/** based on a true story... **/

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

/* function prototypes */
int child();
void parent( int children );

int main()
{
  /* create all the child processes */

  int i, children = 8;  /* based on a true story.... */

  for ( i = 0 ; i < children ; i++ )
  {
    pid_t pid = fork();

    if ( pid < 0 )
    {
      perror( "fork() failed" );
      return EXIT_FAILURE;
    }

    if ( pid == 0 )
    {
      int rc = child();
      exit( rc );   /* return rc; */
    }
  }

#if 1
  parent( children );
#endif

  return EXIT_SUCCESS;
}



/* each child process sleeps for t seconds and returns t
  when it wakes up */
int child()
{
  int t;
  printf( "CHILD: I'm new. My pid is %d.\n", getpid() );

  /* seed the pseudo-random number generator */
  srand( time( NULL ) * getpid() * getpid() );
     /* use the current timestamp */

  t = 10 + ( rand() % 21 );   /* [10,30] */
  printf( "CHILD: I'm gonna nap for %d seconds.\n", t );
  sleep( t );
  return t;
}


void parent( int children )
{
  int status;  /* return status from each child process */
  pid_t child_pid;

#if 0
  printf( "PARENT: I'm gonna nap for 20 seconds.\n" );
  sleep( 20 );
#endif

  printf( "PARENT: I'm waiting for my children to wake up.\n" );

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

  printf( "PARENT: All of my children are awake!\n" );

}
