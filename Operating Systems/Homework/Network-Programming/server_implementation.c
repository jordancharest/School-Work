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

// LOGIN errors
#define ACK "OK\n"
#define EUSRLONG "ERROR: userid is too long\n"
#define EUSRSHRT "ERROR: userid is too short\n"
#define EUSRALNUM "ERROR: userid must be alphanumeric\n"
#define EUSRCONN "ERROR: Already connected\n"

// SEND Errors
#define EUSRUNKNWN "ERROR: Unknown userid\n"
#define EINVMSGLEN "ERROR: Invalid msglen\n"

unsigned int num_active = 0;

// DETERMINE SENDER ==============================================================================
/*  Determine the sender of a request to see if they are logged in                              */
int determine_sender(struct sockaddr_in* client, char* sender) {
    int sender_name_len = 0;

    pthread_mutex_lock(&user_lock);
        for (int i = 0; i < num_active; i++) {

            // sender found
            if (client->sin_addr.s_addr == active_users[i].client->sin_addr.s_addr  &&  client->sin_port == active_users[i].client->sin_port) {
                strcpy(sender, active_users[i].userID);
                sender_name_len = active_users[i].name_len;
                break;
            }
        }
    pthread_mutex_unlock(&user_lock);

    return sender_name_len;   // 0 if no sender is found (must login first)
}

// EXTRACT MESSAGE ===============================================================================
char* extract_message(char* command, char* buffer, int buf_index, int* error, int* full_client_msg_len, char* client_msg_len, int* return_msg_len, char* return_msg) {

    // extract the message length from the command
    int length = 0;
    while (!isspace(buffer[buf_index])  &&  isdigit(buffer[buf_index])  &&  length < 3) {
        client_msg_len[length] = buffer[buf_index];
        buf_index++;
        length++;
    }
    client_msg_len[length] = '\0';
    int cml = atoi(client_msg_len);

    // validate the message length
    if (cml > 994  ||  cml < 1) {
        *return_msg_len = sizeof EINVMSGLEN;
        strncpy(return_msg, EINVMSGLEN, *return_msg_len);
        *error = 1;
    }


    char* client_msg = malloc(cml + 2);
    length = 0;
    buf_index++;

    // extract the client message from the command
    while (buffer[buf_index] != '\0'  &&  length < cml) {
        client_msg[length] = buffer[buf_index];
        buf_index++;
        length++;
    }
    client_msg[length] = '\n';
    client_msg[length+1] = '\0';

    *full_client_msg_len += cml + length;

    return client_msg;
}


// LOGIN VALID USERNAME ==========================================================================
/* A login attempt was made with a valid username (still might be blocked if a TCP client
    already has the username)                                                                   */
void login_valid_username(int socket, struct sockaddr_in* client, char* username, char* msg, int* msg_len, int name_length, char* conn_type) {
    int already_have_username = 0;
    char sender[21];
    int already_logged_in = determine_sender(client, sender);

    // if the sender is asking to log in with the same user name, let them know (a different username is valid)
    if (strcmp(sender, username) == 0)
        already_have_username = 1;

    int user_already_exists = 0;
    int logout_needed = 0;
    struct sockaddr_in* logout_client;
    int* logout_socket = malloc(sizeof *logout_socket);

    // ensure that the username is not taken
    fprintf(stderr, "Taking lock\n");
    pthread_mutex_lock(&user_lock);
    fprintf(stderr, "Successfully took lock\n");
        for (int j = 0; j < num_active; j++) {

            int same_user = strcmp(active_users[j].userID, username);

            // UDP users get preempted, TCP users maintain their connection
            if ((same_user == 0  &&  strcmp(active_users[j].conn_type, "TCP") == 0)  ||  already_have_username) {
                strcpy(msg, EUSRCONN);
                *msg_len = sizeof EUSRCONN;
                printf(EUSRCONN);
                user_already_exists = 1;
                break;

            // UDP: log the previous user out
            } else if (same_user == 0) {
                logout_client = active_users[j].client;
                *logout_socket = active_users[j].socket;
                logout_needed = 1;
            }
        }
    pthread_mutex_unlock(&user_lock);
    fprintf(stderr, "Released lock\n");

    // logout procedure moved outside of mutex to reduce lock time needed
    if (logout_needed) {
        printf("Logout needed\n");
        logout(*logout_socket, logout_client);
        printf("Logout completed\n");
    }
    free(logout_socket);


    // if this IP address and port already has a different username, log the previous username out
    if (already_logged_in  &&  !user_already_exists  &&  strcmp(sender, username) != 0){
        printf("This IP is already logged in, logging out first...\n");
        logout(socket, client);
    }

    // if the username isn't taken, create the user profile and add it to the active users
    if (!user_already_exists) {
        user_t new_user;
        strncpy(new_user.userID, username, name_length+1);

        new_user.active = 1;
        new_user.client = client;
        new_user.socket = socket;
        new_user.name_len = name_length;
        strcpy(new_user.conn_type, conn_type);

        strcpy(msg, ACK);
        *msg_len = sizeof ACK;
        printf("%s has logged in\n", new_user.userID);

        // edit the active user list
        pthread_mutex_lock(&user_lock);
            active_users[num_active] = new_user;
            num_active++;
        pthread_mutex_unlock(&user_lock);
    }
}

// LOGIN =========================================================================================
/*  A login attempt was made (may be invalid)                                                   */
void login_attempt(int socket, struct sockaddr_in* client, char* buffer, char* conn_type) {

    int i = 6;
    int msg_len;
    int length = 0;
    char username[21];
    char msg[64];

    // extract the requested userID
    while (isalnum(buffer[i]) && length < 20) {
        username[length] = buffer[i];
        i++;
        length++;
    }
    username[length] = '\0';


    if (pthread_self() == master_thread)
        printf("MAIN: ");
    else
        printf("CHILD %u: ", (unsigned int)pthread_self());

    printf("Rcvd LOGIN request for userid %s\n", username);


    // validate the userid
    if (buffer[i] != '\n'  &&  !isalnum(buffer[i])) {
        strcpy(msg, EUSRALNUM);
        msg_len = sizeof EUSRALNUM;

    } else if (length == 20) {
        strcpy(msg, EUSRLONG);
        msg_len = sizeof EUSRLONG;

    } else if (length < 3) {
        strcpy(msg, EUSRSHRT);
        msg_len = sizeof EUSRSHRT;

    // username is valid
    } else {
        login_valid_username(socket, client, username, msg, &msg_len, length, conn_type);
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

    // build a string of signed in users
    strcpy(str, ACK);
    int msg_len = sizeof ACK-1; // subtract to account for removing the null terminator with strcat

    // concatenate the active usernames to the string to send
    for (int i = 0; i < num_active; i++) {
        printf("Appending %s to %s", active_users[i].userID, str);
        strcat(str, active_users[i].userID);
        strcat(str, "\n");

        msg_len += (active_users[i].name_len + 1);
    }

    // then send it
    if ( (sendto( socket, str, msg_len, 0, (struct sockaddr* )client, (socklen_t) sizeof(*client) ))  < 0 ) {
        perror("sendto() failed");
    }

    free(str);
}


// LOGOUT ========================================================================================
void logout(int socket, struct sockaddr_in* client) {
    printf("%d requested LOGOUT\n", ntohs(client->sin_port));

    user_t* temp = calloc(MAX_CLIENTS, sizeof *active_users);
    printf("Active users: %d\n", num_active);
    int available_users = num_active;
    for (int i = 0, j = 0; i < available_users; i++) {

        if (client->sin_addr.s_addr == active_users[i].client->sin_addr.s_addr  &&  client->sin_port == active_users[i].client->sin_port)
            num_active--;

        else {
            temp[j] = active_users[i];
            j++;
        }
    }

    free(active_users);
    active_users = temp;

    printf("%d remaining users:\n", num_active);
    for (int i = 0; i < num_active; i++) {
        printf("%s\n", active_users[i].userID);
    }

    if ( (sendto( socket, ACK, sizeof ACK, 0, (struct sockaddr* )client, (socklen_t) sizeof(*client) ))  < 0 ) {
        perror("sendto() failed");
    }

}


// SEND MESSAGE ==================================================================================
void send_msg(int socket, struct sockaddr_in* client, char* buffer) {
    printf("User requested SEND\n");

    int error = 0;

    // figure out who is sending the message
    char sender[21];
    int sender_len = determine_sender(client, sender);


    int i = 5;
    int length = 0;

    int return_msg_len;
    char return_msg[64];

    char recipient[21];
    int recipient_index;

    // extract the recipient username from the command
    while (!isspace(buffer[i]) && length < 20) {
        recipient[length] = buffer[i];
        i++;
        length++;
    }
    recipient[length] = '\0';

    // search for the recipient in the list of active users
    int j = 0;
    for (; j < num_active; j++) {
        if (strcmp(active_users[j].userID, recipient) == 0) {
            return_msg_len = sizeof ACK;
            strncpy(return_msg, ACK, return_msg_len);
            recipient_index = j;
            break;
        } else {
            fprintf(stderr, "%s != %s\n", active_users[j].userID, recipient);
        }
    }


    // if it isn't, tell the sender that the username is unknown
    if (j == num_active) {
        return_msg_len = sizeof EUSRUNKNWN;
        strncpy(return_msg, EUSRUNKNWN, return_msg_len);
        error = 1;
    }


    // extract the message from the sender command
    int buf_index = i+1;
    char client_msg_len[4];
    int full_client_msg_len = 9 + sender_len;   // add 9 to account for spaces/newline/null byte and "FROM"

    char* client_msg = extract_message("SEND", buffer, buf_index, &error, &full_client_msg_len, client_msg_len, &return_msg_len, return_msg);


    // build the full message string
    char* full_client_msg = malloc(full_client_msg_len);
    strcpy(full_client_msg, "FROM ");
    strcat(full_client_msg, sender);
    strcat(full_client_msg, " ");
    strcat(full_client_msg, client_msg_len);
    strcat(full_client_msg, " ");
    strcat(full_client_msg, client_msg);

    // send acknowledgement/error message to the sender
    if ( (sendto( socket, return_msg, return_msg_len, 0, (struct sockaddr* )client, (socklen_t) sizeof(*client) ))  < 0 ) {
        perror("sendto() failed");
    }

    // send the client message to the recipient
    if (!error) {
        if ( (sendto( active_users[recipient_index].socket, full_client_msg, full_client_msg_len, 0, (struct sockaddr* )active_users[recipient_index].client, (socklen_t) sizeof(*(active_users[recipient_index].client)) ))  < 0 ) {
            perror("sendto() failed");
        }
    }


    free(client_msg);
    free(full_client_msg);
}


// BROADCAST =====================================================================================
void broadcast(int socket, struct sockaddr_in* client, char* buffer) {
    printf("User requested BROADCAST\n");

    // figure out who is sending the message
    char sender[21];
    int sender_len = determine_sender(client, sender);

    // extract the message from the sender command
    int error = 0;
    int return_msg_len;
    char client_msg_len[4];
    char return_msg[64];
    int buf_index = 10;
    int full_client_msg_len = 9 + sender_len;   // add 9 to account for spaces/newline/null byte and "FROM"

    char* client_msg = extract_message("SEND", buffer, buf_index, &error, &full_client_msg_len, client_msg_len, &return_msg_len, return_msg);

    // build the full message string
    char* full_client_msg = malloc(full_client_msg_len);
    strcpy(full_client_msg, "FROM ");
    strcat(full_client_msg, sender);
    strcat(full_client_msg, " ");
    strcat(full_client_msg, client_msg_len);
    strcat(full_client_msg, " ");
    strcat(full_client_msg, client_msg);

    // if no parsing error occurred then return an acknowledgment message
    if (!error) {
        return_msg_len = sizeof ACK;
        strncpy(return_msg, ACK, return_msg_len);
    }

    // send acknowledgment/error message to the sender
    if ( (sendto( socket, return_msg, return_msg_len, 0, (struct sockaddr* )client, (socklen_t) sizeof(*client) ))  < 0 ) {
        perror("return sendto() failed");
    }

    // broadcast the client message to all active users
    if (!error) {
        for (int i = 0; i < num_active; i++) {
            if (strcmp(active_users[i].userID, sender) != 0) {
                if ( (sendto( active_users[i].socket, full_client_msg, full_client_msg_len, 0, (struct sockaddr* )active_users[i].client, (socklen_t) sizeof(*(active_users[i].client)) ))  < 0 ) {
                    perror("broadcast sendto() failed");
                }
            }
        }
    }


    free(full_client_msg);
    free(client_msg);
}


// SHARE =========================================================================================
void share(int socket, struct sockaddr_in* client, char* buffer, char* conn_type) {
    printf("User requested SHARE\n");

    //int i = 6;


}

// PARSE COMMAND =================================================================================
void parse_command(int socket, struct sockaddr_in* client, char* buffer, char* conn_type) {

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
        login_attempt(socket, client, buffer, conn_type);

    } else if (strcmp(command, "WHO") == 0) {
        who(socket, client, buffer);

    } else if (strcmp(command, "LOGOUT") == 0) {
        logout(socket, client);

    } else if (strcmp(command, "SEND") == 0) {
        send_msg(socket, client, buffer);

    } else if (strcmp(command, "BROADCAST") == 0) {
        broadcast(socket, client, buffer);

    } else if (strcmp(command, "SHARE") == 0) {
        share(socket, client, buffer, conn_type);

    } else {
        printf("ERROR: unknown command. Valid commands are:\n LOGIN\n WHO\n LOGOUT\n SEND\n BROADCAST\n SHARE\n");
    }



}

