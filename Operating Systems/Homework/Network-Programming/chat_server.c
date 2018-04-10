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

#define MAX_CLIENTS 64
#define MAX_BUFFER 24
#define ACK "OK\n"
#define ACK_LENGTH 3


typedef struct user {
    char userID[21];
    int active;
    struct sockaddr_in* client;
    char con_type[4];
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
    printf("User requested LOGIN\n");

    int i = 6;
    int length = 0;
    char username[21];

    // usernames must be alphanumeric
    while (isalnum(buffer[i]) && length < 20) {
        username[length] = buffer[i];
        i++;
        length++;
    }
    username[length] = '\0';

    // validate
    if (length == 20) {
        printf("ERROR: userid is too long\n");

    } else if (length < 3) {
        printf("ERROR: userid is too short\n");

    } else if (buffer[i] != '\n'  &&  !isalnum(buffer[i])) {
        printf("ERROR: userid must be alphanumeric\n");

    // username is valid
    } else {
        // TODO: only check if a TCP user is already connected
        // ensure that the username is not taken
        int already_connected = 0;
        for (int j = 0; j < num_active; j++) {
            if (strcmp(active_users[j].userID, username) == 0) {
                printf("ERROR: Already connected\n");
                already_connected = 1;
                free(client);
            }
        }

        // if it isn't, create the user profile and add it to the active users
        if (!already_connected) {
            user_t new_user;
            strncpy(new_user.userID, username, length+1);
            new_user.active = 1;
            new_user.client = client;
            strncpy(new_user.con_type, "UDP", 4);

            printf("%s has logged in\n", new_user.userID);

            active_users[num_active] = new_user;
            num_active++;
        }

    }

    printf("There are now %d active users\n", num_active);
}


// WHO ===========================================================================================
void who(struct sockaddr_in* client, char* buffer) {
    printf("User requested WHO\n");

    for (int i = 0; i < num_active; i++) {
        printf("%s\n", active_users[i].userID);
    }
}


// LOGOUT ========================================================================================
void logout(struct sockaddr_in* client, char* buffer) {
    printf("User requested LOGOUT\n");

    //int i = 7;

}


// SEND MESSAGE ==================================================================================
void send_msg(struct sockaddr_in* client, char* buffer) {
    printf("User requested SEND\n");

    //int i = 5;


}


// BROADCAST =====================================================================================
void broadcast(struct sockaddr_in* client, char* buffer) {
    printf("User requested BROADCAST\n");

    //int i = 10;



}


// SHARE =========================================================================================
void share(struct sockaddr_in* client, char* buffer) {
    printf("User requested SHARE\n");

    //int i = 6;


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
    }

    parse_command(client, buffer);


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
