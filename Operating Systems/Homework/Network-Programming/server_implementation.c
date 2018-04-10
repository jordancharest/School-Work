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

#define ACK "OK\n"
#define EUSRLONG "ERROR: userid is too long\n"
#define EUSRSHRT "ERROR: userid is too short\n"
#define EUSRALNUM "ERROR: userid must be alphanumeric\n"
#define EUSRCONN "ERROR: Already connected\n"

unsigned int num_active = 0;


// LOGIN =========================================================================================
void login(int socket, struct sockaddr_in* client, char* buffer) {
    printf("%d requested LOGIN\n", ntohs(client->sin_port));

    int i = 6;
    int msg_len;
    int length = 0;
    char username[21];
    char msg[64];

    // usernames must be alphanumeric
    while (isalnum(buffer[i]) && length < 20) {
        username[length] = buffer[i];
        i++;
        length++;
    }
    username[length] = '\0';

    // validate
    if (buffer[i] != '\n'  &&  !isalnum(buffer[i])) {
        strcpy(msg, EUSRALNUM);
        msg_len = sizeof EUSRALNUM;
        printf(EUSRALNUM);

    } else if (length == 20) {
        strcpy(msg, EUSRLONG);
        msg_len = sizeof EUSRLONG;
        printf(EUSRLONG);

    } else if (length < 3) {
        strcpy(msg, EUSRSHRT);
        msg_len = sizeof EUSRSHRT;
        printf(EUSRSHRT);

    // username is valid
    } else {
        // TODO: only check if a TCP user is already connected
        // ensure that the username is not taken
        int already_connected = 0;
        for (int j = 0; j < num_active; j++) {
            if (strcmp(active_users[j].userID, username) == 0) {
                strcpy(msg, EUSRCONN);
                printf(EUSRCONN);
                already_connected = 1;
            }
        }

        // if it isn't, create the user profile and add it to the active users
        if (!already_connected) {
            user_t new_user;
            strncpy(new_user.userID, username, length+1);
            new_user.active = 1;
            new_user.client = client;
            strncpy(new_user.con_type, "UDP", 4);

            strcpy(msg, ACK);
            msg_len = sizeof ACK;
            printf("%s has logged in\n", new_user.userID);

            active_users[num_active] = new_user;
            num_active++;
        }

    }

    printf("Sending: %s, %d bytes, to client on port %d\n", msg, msg_len, ntohs(client->sin_port));
    if ( (sendto( socket, msg, msg_len, 0, (struct sockaddr* )client, (socklen_t) sizeof(*client) ))  < 0 ) {
        perror("sendto() failed");
    }

    printf("There are now %d active users\n", num_active);
}


// WHO ===========================================================================================
void who(int socket, struct sockaddr_in* client, char* buffer) {
    printf("%d requested WHO\n", ntohs(client->sin_port));

    char* str = malloc(num_active*20);

    // build the list of signed in users

    for (int i = 0; i < num_active; i++) {
        if (i == 0) {
            sprintf(str, "%s\n", ACK);
        }
        sprintf(str, "%s\n", active_users[i].userID);
    }

    // and send it
    if ( (sendto( socket, str, num_active*20, 0, (struct sockaddr* )client, (socklen_t) sizeof(*client) ))  < 0 ) {
        perror("sendto() failed");
    }

    free(str);
}


// LOGOUT ========================================================================================
void logout(struct sockaddr_in* client, char* buffer) {
    printf("%d requested LOGOUT\n", ntohs(client->sin_port));

    user_t temp[64];
    for (int i = 0; i < num_active; i++) {
        if (client->sin_addr.s_addr == active_users[i].client->sin_addr.s_addr  &&  client->sin_port == active_users[i].client->sin_port) {
            num_active--;

        } else {
            temp[i] = active_users[i];
        }
    }

    for (int i = 0; i < num_active; i++) {
        active_users[i] = temp[i];
    }

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
void parse_command(int socket, struct sockaddr_in* client, char* buffer) {

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
        login(socket, client, buffer);

    } else if (strcmp(command, "WHO") == 0) {
        who(socket, client, buffer);

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

