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

// SEND errors
#define EUSRUNKNWN "ERROR: Unknown userid\n"
#define EINVMSGLEN "ERROR: Invalid msglen\n"
#define ELOGINFIRST "ERROR: Must LOGIN first\n"

// SHARE errors
#define ESHAREUDP "SHARE not supported because recipient is using UDP\n"

// Command error
#define EUNKNWNCMD "ERROR: unknown command. Valid commands are:\n LOGIN\n WHO\n LOGOUT\n SEND\n BROADCAST\n SHARE\n"

unsigned int num_active = 0;

// LOG EVENT =====================================================================================
void log_event(char* out) {
    if (master_thread == pthread_self())
        printf("MAIN: ");
    else
        printf("CHILD %u: ", (unsigned int)pthread_self());

    printf("%s", out);
    fflush(stdout);
}

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

// EXTRACT RECIPIENT =============================================================================
/* Extract the recipient of a message from the command                                          */
void extract_recipient(char* buffer, int* buf_index, char* recipient, int* recipient_index) {
    int length = 0;

    while (!isspace(buffer[*buf_index]) && length < 20) {
        recipient[length] = buffer[*buf_index];
        (*buf_index)++;
        length++;
    }
    recipient[length] = '\0';
}


// FIND RECIPIENT ================================================================================
/*  Searches the list of active users by username, returns the socket descriptor and sets the
    sockaddr_in struct pointer to point to the client                                           */
void find_recipient(char* command, char* recipient, struct sockaddr_in* recipient_client,
                    int* recipient_socket, char* return_msg, int* return_msg_len, int* error) {

    // search for the recipient in the list of active users
    for (int j = 0; j < num_active; j++) {

        // if a match is found
        if (strcmp(active_users[j].userID, recipient) == 0) {

            // SHARE is not supported over UDP
            if (strcmp(command, "SHARE") == 0  &&  strcmp(active_users[j].conn_type, "TCP") != 0) {
                *return_msg_len = sizeof ESHAREUDP;
                strncpy(return_msg, ESHAREUDP, *return_msg_len);
                *error = 1;
                log_event(strcat("Sent ", ESHAREUDP));
                break;
            }

            // return the client's info
            *return_msg_len = sizeof ACK;
            strncpy(return_msg, ACK, *return_msg_len);
            *recipient_socket = active_users[j].socket;
            *recipient_client =  *(active_users[j].client);
            return;
        }
    }


    // if it isn't, tell the sender that the username is unknown
    if (*error != 1) {
        *return_msg_len = sizeof EUSRUNKNWN;
        strncpy(return_msg, EUSRUNKNWN, *return_msg_len);
        *error = 1;
        log_event(strcat("Sent ", EUSRUNKNWN));
    }
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
    pthread_mutex_lock(&user_lock);
        for (int j = 0; j < num_active; j++) {

            int same_user = strcmp(active_users[j].userID, username);

            // UDP users get preempted, TCP users maintain their connection
            if ((same_user == 0  &&  strcmp(active_users[j].conn_type, "TCP") == 0)  ||  already_have_username) {
                strcpy(msg, EUSRCONN);
                *msg_len = sizeof EUSRCONN;
                user_already_exists = 1;

                log_event(strcat("Sent ", EUSRCONN));
                break;

            // UDP: log the previous user out
            } else if (same_user == 0) {
                logout_client = active_users[j].client;
                *logout_socket = active_users[j].socket;
                logout_needed = 1;
            }
        }
    pthread_mutex_unlock(&user_lock);

    // logout procedure moved outside of mutex to avoid deadlock (LOGOUT also needs the user_lock)
    if (logout_needed) {
        logout(*logout_socket, logout_client);
    }
    free(logout_socket);


    // if this IP address and port already has a different username, log the previous username out
    if (already_logged_in  &&  !user_already_exists  &&  strcmp(sender, username) != 0){
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


    char request[64];
    sprintf(request, "Rcvd LOGIN request for userid %s\n", username);
    log_event(request);


    // validate the userid
    if (buffer[i] != '\n'  &&  !isalnum(buffer[i])) {
        strcpy(msg, EUSRALNUM);
        msg_len = sizeof EUSRALNUM;
        log_event(strcat("Sent ", EUSRALNUM));

    } else if (length == 20) {
        strcpy(msg, EUSRLONG);
        msg_len = sizeof EUSRLONG;
        log_event(strcat("Sent ", EUSRLONG));

    } else if (length < 3) {
        strcpy(msg, EUSRSHRT);
        msg_len = sizeof EUSRSHRT;
        log_event(strcat("Sent ", EUSRSHRT));

    // username is valid
    } else {
        login_valid_username(socket, client, username, msg, &msg_len, length, conn_type);
    }

    // printf("Sending: %s, %d bytes, to client on port %d\n", msg, msg_len, ntohs(client->sin_port));
    if ( (sendto( socket, msg, msg_len, 0, (struct sockaddr* )client, (socklen_t) sizeof(*client) ))  < 0 ) {
        perror("sendto() failed");
    }
}


// WHO ===========================================================================================
/*  Sends a list of active users to the requester (works even if requester is not logged in)    */
void who(int socket, struct sockaddr_in* client, char* buffer) {

    log_event("Rcvd WHO request\n");


    char* str = malloc(num_active*20);

    // build a string of signed in users
    strcpy(str, ACK);
    int msg_len = sizeof ACK-1; // subtract 1 to account for removing the null terminator with strcat

    // concatenate the active usernames to the string to send
    for (int i = 0; i < num_active; i++) {
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

    log_event("Rcvd LOGOUT request\n");


    user_t* temp = calloc(MAX_CLIENTS, sizeof *temp);

    // determine who is requesting logout, remove them from active users list
    pthread_mutex_lock(&user_lock);
        int available_users = num_active;
        for (int i = 0, j = 0; i < available_users; i++) {

            if (client->sin_addr.s_addr == active_users[i].client->sin_addr.s_addr  &&  client->sin_port == active_users[i].client->sin_port) {
                num_active--;
                free(active_users[i].client);

            } else {
                temp[j] = active_users[i];
                j++;
            }
        }
        free(active_users);
        active_users = temp;
    pthread_mutex_unlock(&user_lock);
}


// SEND MESSAGE ==================================================================================
void send_msg(int socket, struct sockaddr_in* client, char* buffer) {

    int buf_index = 5;
    int return_msg_len;
    char return_msg[64];

    char recipient[21];
    int recipient_index;

    // extract the name of the recipient from the command
    extract_recipient(buffer, &buf_index, recipient, &recipient_index);

    char request[64];
    sprintf(request, "Rcvd SEND request to userid %s\n", recipient);
    log_event(request);


    int error = 0;

    // figure out who is sending the message
    char sender[21];
    int sender_len = determine_sender(client, sender);

    // if sender is unknown, they must LOGIN first
    if (sender_len == 0) {
        if ( (sendto( socket, ELOGINFIRST, sizeof ELOGINFIRST, 0, (struct sockaddr* )client, (socklen_t) sizeof(*client) ))  < 0 ) {
            perror("sendto() failed");
        }

        return;
    }


    // find the recipient in the list of active users
    int recipient_socket;
    struct sockaddr_in* recipient_client = malloc(sizeof *recipient_client);
    find_recipient("SEND", recipient, recipient_client, &recipient_socket, return_msg, &return_msg_len, &error);


    // extract the message from the sender command
    buf_index++;
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
        if ( (sendto( recipient_socket, full_client_msg, full_client_msg_len, 0, (struct sockaddr* )recipient_client, (socklen_t) sizeof(*(active_users[recipient_index].client)) ))  < 0 ) {
            perror("sendto() failed");
        }
    }

    free(client_msg);
    free(full_client_msg);
    free(recipient_client);
}


// BROADCAST =====================================================================================
void broadcast(int socket, struct sockaddr_in* client, char* buffer) {

    log_event("Rcvd BROADCAST request\n");

    // figure out who is sending the message
    char sender[21];
    int sender_len = determine_sender(client, sender);

    // if sender is unknown, they must LOGIN first
    if (sender_len == 0) {
        if ( (sendto( socket, ELOGINFIRST, sizeof ELOGINFIRST, 0, (struct sockaddr* )client, (socklen_t) sizeof(*client) ))  < 0 ) {
            perror("sendto() failed");
        }

        return;
    }

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

    log_event("Rcvd SHARE request\n");


    int error = 0;

    // SHARE is not supported over UDP
    if (strcmp("TCP", conn_type) != 0) {
        printf("SHARE not supported over UDP\n");
        return;
    }

    // figure out who is sending the message
    char sender[21];
    int sender_len = determine_sender(client, sender);

    // if sender is unknown, they must LOGIN first
    if (sender_len == 0) {
        if ( (sendto( socket, ELOGINFIRST, sizeof ELOGINFIRST, 0, (struct sockaddr* )client, (socklen_t) sizeof(*client) ))  < 0 ) {
            perror("sendto() failed");
        }

        return;
    }

    int buf_index = 6;
    char recipient[21];
    int recipient_index;

    // extract the recipient's name from the received command
    extract_recipient(buffer, &buf_index, recipient, &recipient_index);

    // find recipient in the list of active users
    int recipient_socket;
    struct sockaddr_in* recipient_client = malloc(sizeof *recipient_client);
    int return_msg_len;
    char return_msg[64];
    find_recipient("SHARE", recipient, recipient_client, &recipient_socket, return_msg, &return_msg_len, &error);

    // don't need the client to send over TCP
    free(recipient_client);

    // extract the file length
    buf_index++;
    int length = 0;
    char client_msg_len[16];

    while (!isspace(buffer[buf_index])  &&  isdigit(buffer[buf_index])) {
        client_msg_len[length] = buffer[buf_index];
        buf_index++;
        length++;
    }
    client_msg_len[length] = '\0';
    int remaining_bytes = atoi(client_msg_len);

    // validate the message length (no max file length)
    if (remaining_bytes < 1) {
        return_msg_len = sizeof EINVMSGLEN;
        strncpy(return_msg, EINVMSGLEN, return_msg_len);
        error = 1;
        log_event(strcat("Sent ", EINVMSGLEN));
    }

    // Let sender know if there was an error or acknowledge success
    if (error) {
        send(socket, return_msg, return_msg_len, 0);
        return;
    } else
        send(socket, ACK, sizeof ACK, 0);


    // let the recipient know someone shared a file with them
    char recipient_msg[32];
    int recipient_msg_len = 8 + sender_len + length;

    strcpy(recipient_msg, "FROM ");
    strcat(recipient_msg, sender);
    strcat(recipient_msg, " ");
    strcat(recipient_msg, client_msg_len);

    recipient_msg[recipient_msg_len-1] = '\n';

    if (send(recipient_socket, recipient_msg, recipient_msg_len, 0) != recipient_msg_len) {
        perror("send() failed");
        exit(EXIT_FAILURE);
    }


    // receive and send the file in 1024 byte chunks
    char file_buffer[1024];     // received messages are not expected to be characters (i.e. readable text)
    while (remaining_bytes > 0) {

        // recv bytes from sender
        int n_recv = recv(socket, file_buffer, MAX_BUFFER, 0 );
        if (n_recv < 0) {
            perror("recv() failed");
            exit(EXIT_FAILURE);

        } else if (n_recv == 0) {
            char log_msg[64];
            sprintf(log_msg, "CHILD %u: Client disconnected\n", (unsigned int)pthread_self());
            log_event(log_msg);
            break;

        // send bytes to recipient
        } else {

            int n_sent = send(recipient_socket, file_buffer, n_recv, 0);
            if (n_sent != n_recv) {
                perror("send() failed");
                exit(EXIT_FAILURE);
            }

            // Acknowledge the sender
            int n = send(socket, ACK, sizeof ACK, 0);
            if (n != sizeof ACK) {
                perror("send() failed");
                exit(EXIT_FAILURE);
            }

            remaining_bytes -= n_recv;
        }
    }
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

        // User logins may sometimes prompt a logout, so move acknowledgment outside logout function to prevent message duplicates
        if ( ( sendto(socket, ACK, sizeof ACK, 0, (struct sockaddr* )client, (socklen_t) sizeof *client) ) < 0 )
            perror("sendto() failed");

    } else if (strcmp(command, "SEND") == 0) {
        send_msg(socket, client, buffer);

    } else if (strcmp(command, "BROADCAST") == 0) {
        broadcast(socket, client, buffer);

    } else if (strcmp(command, "SHARE") == 0) {
        share(socket, client, buffer, conn_type);

    } else {
        log_event(strcat("Sent ", EUNKNWNCMD));
    }
}

