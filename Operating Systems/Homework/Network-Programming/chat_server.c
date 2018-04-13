#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <string.h>

#include "server_implementation.h"

typedef struct thread_args {
    int socket;
    struct sockaddr_in* client;
} TA_t;

pthread_t master_thread;


// UDP INIT ======================================================================================
int UDP_Init(struct sockaddr_in* server, int port) {
    int sd;
                    /* IPv4,      UDP, default */
    if ( (sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    server->sin_family = AF_INET;   // IPv4
    server->sin_addr.s_addr = htonl(INADDR_ANY);

    // specify the port number for the server
    server->sin_port = htons(port);

    // bind to user assigned port number
    if ( bind (sd, (struct sockaddr* )server, sizeof(*server)) < 0) {
        perror("bind() failed");
        exit(EXIT_FAILURE);
    }

    /*
    int length = sizeof(*server);
    if ( getsockname(sd, (struct sockaddr *)server, (socklen_t *)&length) < 0 ) {
        perror("getsockname() failed");
        exit(EXIT_FAILURE);
    }
*/
    printf("MAIN: Listening for UDP datagrams on port: %d\n", ntohs(server->sin_port));
    fflush(stdout);

    return sd;
}


// HANDLE UDP DATAGRAM ===========================================================================
void handle_UDP_datagram(int UDP_socket, fd_set* read_fd_set) {

    struct sockaddr_in* client = malloc(sizeof *client);
    int len = sizeof client;
    char buffer[MAX_BUFFER];

    int n_bytes = recvfrom(UDP_socket, buffer, MAX_BUFFER, 0, (struct sockaddr* )client, (socklen_t* )&len);
    if (n_bytes < 0) {
        perror("recvfrom() failed");

    } else {
        printf("MAIN: Rcvd incoming UDP datagram from: %s\n", inet_ntoa(client->sin_addr));
        buffer[n_bytes] = '\0';

        parse_command(UDP_socket, client, buffer, "UDP");
    }

    // clear the bit flag for this file descriptor
    //FD_CLR(UDP_socket, read_fd_set);
}


// TCP INIT ======================================================================================
int TCP_Init(struct sockaddr_in* server, int port) {
    int sd;

    // Create the listener socket as TCP socket
    if ( (sd = socket( PF_INET, SOCK_STREAM, 0 )) < 0 ) {       // note that PF_INET is protocol family, Internet
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    server->sin_family = AF_INET;
    server->sin_addr.s_addr = htonl(INADDR_ANY);    // allow any IP address to connect

    server->sin_port = htons(port);    // assign the port number specified in command line argument

    // bind to user assigned port number
    if ( bind(sd, (struct sockaddr* )server, sizeof(*server)) < 0) {
        perror("bind() failed");
        exit(EXIT_FAILURE);
    }

    // identify the port as a listener
    if ( listen(sd, 5) < 0) {
        perror("listen() failed");
        exit(EXIT_FAILURE);
    }

    printf("MAIN: Listening for TCP connections on port: %d\n", ntohs(server->sin_port));
    fflush(stdout);

    return sd;
}


// HANDLE TCP CONNECTION =========================================================================
/*  Thread function                                                                             */
void* handle_TCP_connection (void* args) {

    // automatically clean up thread resources
    pthread_detach(pthread_self());

    TA_t* thread_args = (TA_t*)args;
    int socket = thread_args->socket;
    struct sockaddr_in* client = thread_args->client;

    char buffer[MAX_BUFFER];

    // make a new local fd_set; only care about the socket descriptor, and don't want to synchronize with main thread
    fd_set read_fd_set;

    // loop until the connection is closed
    while (1) {
        FD_CLR(socket, &read_fd_set);
        FD_SET(socket, &read_fd_set);

        if ( FD_ISSET(socket, &read_fd_set) ) {
            int n_bytes = recv(socket, buffer, MAX_BUFFER, 0);
            if (n_bytes < 0) {
                perror("recv() failed");

            // client closed the connection
            } else if (n_bytes == 0) {
                printf("Client closed the connection...closing socket\n");
                logout(socket, client);
                break;

            } else {
                buffer[n_bytes] = '\0';

                parse_command(socket, client, buffer, "TCP");
            }
        }
    }


    return NULL;
}

// MAIN ==========================================================================================
int main(int argc, char** argv) {

    if (argc != 3) {
        fprintf(stderr, "ERROR: invalid argument(s)\nUSAGE: %s <TCP-port> <UDP-port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("MAIN: Started server\n");
    master_thread = pthread_self();
    active_users = calloc(MAX_CLIENTS, sizeof *active_users);

    struct sockaddr_in TCP_server;
    int TCP_listener = TCP_Init(&TCP_server, atoi(argv[1]));

    struct sockaddr_in UDP_server;
    int UDP_socket = UDP_Init(&UDP_server, atoi(argv[2]));

    fd_set read_fd_set;

    while (1) {
        struct timeval timeout;
        timeout.tv_sec = 60;
        timeout.tv_usec = 500;  // 60 AND 500 microseconds

        // reset the fd set for TCP listener and UDP
        FD_ZERO(&read_fd_set);
        FD_SET(UDP_socket, &read_fd_set);
        FD_SET(TCP_listener, &read_fd_set);

        // look for UDP datagrams or TCP connection requests
        int ready = select(FD_SETSIZE, &read_fd_set, NULL, NULL, &timeout);
        if (ready == 0) {
            printf("No activity\n");
            continue;

        // at least one connection occurred
        } else {
            // check for UDP datagram
            if (FD_ISSET(UDP_socket, &read_fd_set))
                handle_UDP_datagram(UDP_socket, &read_fd_set);

            // check for new TCP connection requests on the listener
            if (FD_ISSET(TCP_listener, &read_fd_set)) {
                struct sockaddr_in* client = malloc(sizeof *client);
                int len = sizeof *client;

                int new_socket = accept(TCP_listener, (struct sockaddr* )client, (socklen_t* )&len);
                if (new_socket < 0)
                    perror("accept() failed");

                // connection established
                else {
                    printf("MAIN: Rcvd incoming TCP_connection from: %s\n", inet_ntoa(client->sin_addr));
                    fflush(stdout);

                    pthread_t tid;
                    TA_t send_args;
                    send_args.socket = new_socket;
                    send_args.client = client;

                    // offload the TCP connection to a new thread
                    if ( pthread_create( &tid, NULL, handle_TCP_connection, (void*)&send_args ) != 0 ) {
                        fprintf( stderr, "ERROR: Could not create thread\n" );
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }
    }

    free(active_users);

    return EXIT_SUCCESS;
}
