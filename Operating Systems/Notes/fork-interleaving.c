/* fork-interleaving.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
  pid_t pid_rc;   /* process id (pid_t is unsigned short) */

  printf( "Welcome to the fork-interleaving.c example\n" );

  /* create a new (child) process */
  pid_rc = fork();

  printf( "After fork()\n" );

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
sleep( 30 );
#endif
    return 255;  /* this must be one byte */
  }
  else if ( pid_rc > 0 )  /* PARENT PROCESS */
  {
    printf( "PARENT: my child process has pid %d\n", pid_rc );
    printf( "PARENT: my pid is %d\n", getpid() );
#if 0
sleep( 30 );
#endif

    printf( "PARENT: waiting for my child process to terminate...\n" );

    int status;
#if 0
    pid_t child_pid = wait( &status );   /* wait() blocks indefinitely */
    pid_t child_pid = waitpid( pid_rc, &status, 0 );
    pid_t child_pid = waitpid( pid_rc, &status, WNOHANG );  /* will not block */
#endif
    pid_t child_pid = waitpid( -1, &status, 0 );

    printf( "PARENT: child process %d terminated...\n", child_pid );

    if ( WIFSIGNALED( status ) )  /* child process was terminated due to */
    {                             /*  a signal (e.g., segfault, etc.)    */
      printf( "...abnormally\n" );
    }
    else if ( WIFEXITED( status ) )  /* child process returned or exit() */
    {
      int child_return_value = WEXITSTATUS( status );
      printf( "...successfully with exit status %d\n", child_return_value );
    }
  }

  return EXIT_SUCCESS;
}
