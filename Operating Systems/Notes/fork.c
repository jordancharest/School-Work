/* fork.c */

#if 0

  Each process has a unique id called a process ID (pid)

  bash$ ps -ef | less

  bash$ ps -ef | grep goldsd

  All processes form a process tree


  do not do this:
    while ( 1 ) { fork(); }

#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
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
#if 0
sleep( 20 );
#endif
  }
  else if ( pid_rc > 0 )  /* PARENT PROCESS */
  {
    printf( "PARENT: my child process has pid %d\n", pid_rc );
    printf( "PARENT: my pid is %d\n", getpid() );
  }

  /* both parent and child processes also run this code below: */
#if 0
  sleep( 30 );  /* sleep for 30 seconds */
#endif

  return EXIT_SUCCESS;
}
