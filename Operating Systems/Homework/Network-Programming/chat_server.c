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
    server->sin_port = htons(port); // 0 means let the kernel assign us a port number

    // bind to OS-assigned port number
    if ( bind (sd, (struct sockaddr* )server, sizeof(*server)) < 0) {
        perror("bind() failed");
        exit(EXIT_FAILURE);
    }

    int length = sizeof(*server);
    if ( getsockname(sd, (struct sockaddr *)server, (socklen_t *)&length) < 0 ) {
        perror("getsockname() failed");
        exit(EXIT_FAILURE);
    }

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
        perror("recvfrom() failed\n");

    } else {
        printf( "Rcvd  %d byte datagram from %s port %d\n", n_bytes, inet_ntoa( client->sin_addr ), ntohs(client->sin_port) );
        buffer[n_bytes] = '\0';
        printf("RCVD: %s\n", buffer);

        parse_command(UDP_socket, client, buffer);
    }

    // clear the bit flag for this file descriptor
    FD_CLR(UDP_socket, read_fd_set);
}


// MAIN ==========================================================================================
int main(int argc, char** argv) {

    if (argc != 2) {
        fprintf(stderr, "ERROR: invalid argument(s)\nUSAGE: %s <UDP_Port>\n", argv[0]);
        return EXIT_FAILURE;
    }


    printf("MAIN: Started server\n");
    printf("MAIN: Listening for TCP connections on port: <nothing>\n");

    struct sockaddr_in server;
    int UDP_socket = UDP_Init(&server, atoi(argv[1]));

    fd_set read_fd_set;

    //int client_sockets[MAX_CLIENTS];
    //int client_socket_index = 0;


    while (1) {
        struct timeval timeout;
        timeout.tv_sec = 60;
        timeout.tv_usec = 500;  // 60 AND 500 microseconds

        FD_ZERO(&read_fd_set);
        FD_SET(UDP_socket, &read_fd_set);

        /*
        for (int i = 0; i < client_socket_index; i++) {
            FD_SET(client_sockets[ i ], &read_fd_set);
            printf("Set FD_SET to include client socket fd %d\n", client_sockets[i]);
        }
        */
        // look for UDP datagrams or TCP connection requests
        int ready = select(FD_SETSIZE, &read_fd_set, NULL, NULL, &timeout);
        if (ready == 0) {
            printf("No activity\n");
            continue;

        // Received a UDP datagram
        } else if (FD_ISSET(UDP_socket, &read_fd_set)) {
            handle_UDP_datagram(UDP_socket, &read_fd_set);
        }
    }

    return EXIT_SUCCESS;
}
