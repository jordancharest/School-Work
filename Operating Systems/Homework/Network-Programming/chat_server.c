#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <string.h>

#define MAX_CLIENTS 64
#define MAX_BUFFER 24
#define ACK "OK\n"
#define ACK_LENGTH 3


typedef struct user {
    char userID[21];
    int active;
    char address[20];
} user_t;

user_t active_users[64];
unsigned int num_active = 0;

// UDP INIT ======================================================================================
int UDP_Init(struct sockaddr_in* server) {
    int sd;
                    /* IPv4,      UDP, default */
    if ( (sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    server->sin_family = AF_INET;   // IPv4
    server->sin_addr.s_addr = htonl(INADDR_ANY);

    // specify the port number for the server
    server->sin_port = htons(12345); // 0 means let the kernel assign us a port number

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

    printf("UDP Server initialized at port number %d\n", ntohs(server->sin_port));

    return sd;
}

// LOGIN =========================================================================================
void login(struct sockaddr_in* client, char* buffer) {





}


// WHO ===========================================================================================
void who(struct sockaddr_in* client, char* buffer) {





}


// LOGOUT ========================================================================================
void logout(struct sockaddr_in* client, char* buffer) {





}


// SEND MESSAGE ==================================================================================
void send_msg(struct sockaddr_in* client, char* buffer) {





}


// BROADCAST =====================================================================================
void broadcast(struct sockaddr_in* client, char* buffer) {





}


// SHARE =========================================================================================
void share(struct sockaddr_in* client, char* buffer) {





}

// PARSE COMMAND =================================================================================
void parse_command(struct sockaddr_in* client, char* buffer) {

    // extract the command
    int i = 0;
    char command[10];
    while (buffer[i] != ' ' && buffer[i] != '\n') {
        command[i] = buffer[i];
        i++;
    }

    command[i] = '\0';

    // fulfill the command
    if (strcmp(command, "LOGIN") == 0) {
        login(client, buffer);

    } else if (strcmp(command, "WHO") == 0) {
        who(client, buffer);

    } else if (strcmp(command, "LOGOUT") == 0) {
        logout(client, buffer);

    } else if (strcmp(command, "SEND") == 0) {
        send_msg(client, buffer);

    } else if (strcmp(command, "BROADCAST") == 0) {
        broadcast(client, buffer);

    } else if (strcmp(command, "SHARE") == 0) {
        share(client, buffer);

    } else {
        printf("ERROR: unknown command. Valid commands are:\n LOGIN\n WHO\n LOGOUT\n SEND\n BROADCAST\n SHARE\n");
    }



}


// HANDLE UDP DATAGRAM ===========================================================================
void handle_UDP_datagram(int UDP_socket, fd_set* read_fd_set) {

    struct sockaddr_in client;
    int len = sizeof client;
    char buffer[MAX_BUFFER];

    int n_bytes = recvfrom(UDP_socket, buffer, MAX_BUFFER, 0, (struct sockaddr* )&client, (socklen_t* )&len);
    if (n_bytes < 0) {
        perror("recvfrom() failed\n");

    } else {
        printf( "Rcvd  %d byte datagram from %s port %d\n", n_bytes, inet_ntoa( client.sin_addr ), ntohs(client.sin_port) );
        buffer[n_bytes] = '\0';
        printf("RCVD: %s\n", buffer);
    }

    parse_command(&client, buffer);


    // clear the bit flag for this file descriptor
    FD_CLR(UDP_socket, read_fd_set);
}


// MAIN ==========================================================================================
int main(int argc, char** argv) {

    struct sockaddr_in server;
    int UDP_socket = UDP_Init(&server);

    fd_set read_fd_set;

    //int client_sockets[MAX_CLIENTS];
    //int client_socket_index = 0;


    while (1) {
        struct timeval timeout;
        timeout.tv_sec = 60;
        timeout.tv_usec = 500;  // 3 AND 500 microseconds

        FD_ZERO(&read_fd_set);
        FD_SET(UDP_socket, &read_fd_set);
        //printf("Set FD_SET to include fd %d\n", UDP_socket);

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
