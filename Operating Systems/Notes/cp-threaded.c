/* cp-threaded.c */

/* This will copy one or more files, as specified on the command-line
 * (using a separate thread to copy each one).
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

/* gcc ... -lpthread */
#include <pthread.h>

#define BUFSIZE     256
#define MAXLEN      128
#define MAXTHREADS  100

void * copy_file( void * arg );

int main( int argc, char * argv[] )
{
  pthread_t tid[ MAXTHREADS ];

  int total_bytes_copied = 0;
  int * bytes_copied_p;
  int i;
  int rc;

  for ( i = 1 ; i < argc ; i++ )
  {
    printf( "MAIN: Creating thread to copy %s\n", argv[i] );
    rc = pthread_create( &tid[i], NULL, copy_file, (void *)argv[i] );
    if ( rc != 0 )
    {
      fprintf( stderr, "MAIN: Could not create thread %d\n", i );
      fprintf( stderr, "MAIN: rc is %d\n", rc );
    }
  }

  /* wait for the threads to complete */
  for ( i = 1 ; i < argc ; i++ )
  {
    rc = pthread_join( tid[i], (void **)&( bytes_copied_p ) );
    if ( rc != 0 )
    {
      fprintf( stderr, "MAIN: No thread %d to join\n", i );
    }
    else
    {
      printf( "MAIN: Thread %d copied %d bytes from %s\n",
              tid[i], *bytes_copied_p, argv[i] );
      total_bytes_copied += *bytes_copied_p;
    }
  }

  printf( "MAIN: Total bytes copied is %d\n", total_bytes_copied );

  return 0;
}

void * copy_file( void * arg )
{
  int infile, outfile;
  int bytes_read = 0;
  int bytes_written = 0;
  char buffer[ BUFSIZE ];
  char outfilename[ MAXLEN ];
  int * rc = (int *)malloc( sizeof ( int ) );
  *rc = 0;

  printf( "THREAD %d: Copying %s...\n", pthread_self(), (char *)arg );

  infile = open( arg, O_RDONLY );
  if ( infile < 0 )
  {
    fprintf( stderr, "THREAD %d: Error opening file %s: ",
             pthread_self(), (char *)arg );
    pthread_exit( rc );
  }

  strcpy( outfilename, "backup_" );
  strcat( outfilename, arg ); 
  outfile = open( outfilename, O_WRONLY | O_CREAT | O_TRUNC, 
                  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );
  if ( outfile < 0 )
  {
    fprintf( stderr, "THREAD %d: Error opening file %s",
             pthread_self(), outfilename );
    pthread_exit( rc );
  }

  while ( 1 )
  {
    bytes_read = read( infile, buffer, BUFSIZE );
    if ( bytes_read == 0 )
    {
      break;
    }
    else if ( bytes_read < 0 )
    {
      perror( "reading" );
      pthread_exit( rc );
    }
    bytes_written = write( outfile, buffer, bytes_read );
    if ( bytes_written != bytes_read )
    {
      perror( "writing" );
      pthread_exit( rc );
    }
    *rc += bytes_written;
  }

  close( infile );
  close( outfile );
  pthread_exit( rc );
  return rc;  /* we never get here ... */
}
