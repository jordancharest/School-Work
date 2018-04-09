/* udp-client.c */

/* Simple UDP echo client - tries to send everything read from stdin
   as a single datagram */

/* Run this as follows:

   bash$ ./udp-client <hostname> <port>

   If running on the same machine as the server:

   bash$ ./udp-client localhost <port>

   or...

   bash$ hostname
   <some-hostname-xyz>
   bash$ ./udp-client <some-hostname-xyz> <port>

   can also use netcat or nc:

   bash$ nc -u <some-hostname-xyz> <port>
*/

#include <stdio.h>      /* standard C i/o facilities */
#include <stdlib.h>     /* needed for atoi() */
#include <unistd.h>     /* defines STDIN_FILENO, system calls,etc */
#include <sys/types.h>  /* system data type definitions */
#include <sys/socket.h> /* socket specific definitions */
#include <netinet/in.h> /* INET constants and stuff */
#include <arpa/inet.h>  /* IP address conversion stuff */
#include <netdb.h>      /* gethostbyname */
#include <string.h>

/* get_stdin reads from standard input until EOF is found,
   or the maximum bytes have been read.
*/

int get_stdin( char * buffer, int maxlen )
{
  int n, i = 0;

  while ( ( n = read( STDIN_FILENO, buffer + i, maxlen - i ) ) > 0 )
  {
    i += n;
    if ( i == maxlen ) break;
  }

  if ( n != 0 )
  {
    perror( "Error reading stdin" );
    exit( EXIT_FAILURE );
  }

  /* return the number of bytes read including the last read */
  return i;
}


/* client program:

   The following must passed in on the command line:
      hostname of the server (argv[1])
      port number of the server (argv[2])
*/

#define MAXBUF 10*1024

int main( int argc, char *argv[] )
{
  int sd, length, n_sent, n_read;
  struct sockaddr_in server;
  struct hostent * hp;
  char buffer[MAXBUF];

  if ( argc != 3 )
  {
    fprintf( stderr, "USAGE: %s <server-name> <port-number>\n", argv[0] );
    return EXIT_FAILURE;
  }

  /* create a socket
     IP protocol family (PF_INET)
     UDP (SOCK_DGRAM)
  */

  if ( ( sd = socket( PF_INET, SOCK_DGRAM, 0 ) ) < 0 )
  {
    perror( "socket() failed" );
    return EXIT_FAILURE;
  }

  /* Using UDP we don't need to call bind unless we care what our
     port number is - clients shouldn't care */

  /* now create a sockaddr that will be used to contact the server

     fill in an address structure that will be used to specify
     the address of the server we want to connect to

     address family is IP  (AF_INET)

     server IP address is found by calling gethostbyname with the
     name of the server (entered on the command line)

     server port number is argv[2] (entered on the command line)
  */

  server.sin_family = AF_INET;

  if ( ( hp = gethostbyname( argv[1] ) ) == NULL )
  {
    perror( "gethostbyname() failed" );
    return EXIT_FAILURE;
  }

  /* copy the IP address into the sockaddr
     It is already in network byte order
  */

  memcpy( &server.sin_addr.s_addr, hp->h_addr, hp->h_length);

  /* establish the server port number - we must use network byte order! */
  server.sin_port = htons( atoi ( argv[2] ) );

  /* read everything possible */
  length = get_stdin( buffer, MAXBUF );
  printf( "Got %d bytes from stdin - sending...\n", length );

  /* send it to the echo server */
  n_sent = sendto( sd, buffer, length, 0,
                   (struct sockaddr *) &server, sizeof( server ) );

  if ( n_sent < 0 )
  {
    perror( "sendto() failed" );
    return EXIT_FAILURE;
  }

  if ( n_sent != length )
  {
    printf( "sendto() sent %d bytes\n", n_sent );
  }

  /* Wait for a reply (from anyone...) */
  n_read = recvfrom( sd, buffer, MAXBUF, 0, NULL, NULL );

  if ( n_read < 0 )
  {
    perror( "recvfrom() failed" );
    return EXIT_FAILURE;
  }

  printf( "Rcvd %d bytes\n", n_read );

  /* send what we got back to stdout */
  if ( write( STDOUT_FILENO, buffer, n_read ) < 0 )
  {
    perror( "write() failed" );
    return EXIT_FAILURE;
  }

  close( sd );

  return EXIT_SUCCESS;
}

