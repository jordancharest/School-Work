/* signal-input.c */

/* check out: "man 7 signal" */
/* check out: "man 2 signal" */
/* check out: "man sigaction" */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

void signal_handler( int n )
{
  char output[100];
  sprintf( output, "Rcvd signal %d\n", n );
  write( 1, output, strlen( output ) );

  if ( n == SIGINT )
  {
    printf( "Stop hitting CTRL-C and answer the question!\n" );
  }
  else if ( n == SIGUSR1 )
  {
    printf( "Reloading the config file......\n" );
  }
}

int main()
{
  signal( SIGINT, SIG_IGN );   /* ignore SIGINT */
  signal( SIGINT, SIG_DFL );   /* restore the default behavior for the signal */

  /* redefine SIGINT to call signal_handler() */
  signal( SIGINT, signal_handler );
  signal( SIGUSR1, signal_handler );

  char name[100];
  printf( "Enter your name: " );
  scanf( "%s", name );
  printf( "Hi, %s\n", name );

  return EXIT_SUCCESS;
}
