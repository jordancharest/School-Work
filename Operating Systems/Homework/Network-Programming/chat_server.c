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
    printf("MAIN: Listening for UDP datagrams on port: %d", ntohs(server->sin_port));
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
        printf( "Rcvd  %d byte datagram from %s port %d\n", n_bytes, inet_ntoa( client->sin_addr ), ntohs(client->sin_port) );
        buffer[n_bytes] = '\0';
        printf("RCVD: %s\n", buffer);

        parse_command(UDP_socket, client, buffer);
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

    server->sin_family = PF_INET;
    server->sin_addr.s_addr = htonl(INADDR_ANY);    // allow any IP address to connect

    server->sin_port = port;    // assign the port number specified in command line argument

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

    printf("MAIN: Listening for TCP connections on port: %d", ntohs(server->sin_port));
    fflush(stdout);

    return sd;
}


// HANDLE TCP CONNECTION =========================================================================
void* handle_TCP_connection (void* args) {




    return NULL;
}

// MAIN ==========================================================================================
int main(int argc, char** argv) {

    if (argc != 3) {
        fprintf(stderr, "ERROR: invalid argument(s)\nUSAGE: %s <TCP-port> <UDP-port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("MAIN: Started server\n");
    active_users = calloc(MAX_CLIENTS, sizeof *active_users);

    struct sockaddr_in TCP_server;
    int TCP_listener = TCP_Init(&TCP_server, atoi(argv[1]));

    struct sockaddr_in UDP_server;
    int UDP_socket = UDP_Init(&UDP_server, atoi(argv[2]));

    fd_set read_fd_set;

    int client_sockets[MAX_CLIENTS];
    int client_socket_index = 0;

    while (1) {
        struct timeval timeout;
        timeout.tv_sec = 60;
        timeout.tv_usec = 500;  // 60 AND 500 microseconds

        // reset the fd set for TCP listener and UDP
        FD_ZERO(&read_fd_set);
        FD_SET(UDP_socket, &read_fd_set);
        FD_SET(TCP_listener, &read_fd_set);

        // reset the fd's for all connected TCP clients
        for (int i = 0; i < client_socket_index; i++) {
            FD_SET(client_sockets[ i ], &read_fd_set);
            printf("Set FD_SET to include client socket fd %d\n", client_sockets[i]);
        }

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

            // check for new connection requests on the listener
            if (FD_ISSET(TCP_listener, &read_fd_set)) {
                struct sockaddr_in* client = malloc(sizeof *client);
                int len = sizeof *client;
                int new_socket = accept(TCP_listener, (struct sockaddr* )client, (socklen_t* )&len);

                // offload the TCP connection to a new thread
                // pthread_create...
            }
        }
    }

    free(active_users);

    return EXIT_SUCCESS;
}
