#ifndef SERVER_IMPLEMENTATION_H_INCLUDED
#define SERVER_IMPLEMENTATION_H_INCLUDED

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
#define ACK_LENGTH 4


// GLOBAL ========================================================================================
typedef struct user {
    char userID[21];
    int active;
    struct sockaddr_in* client;
    char con_type[4];
} user_t;

user_t active_users[64];
extern unsigned int num_active;


// FUNCTION PROTOTYPES ===========================================================================
void login(struct sockaddr_in* client, char* buffer);
void who(struct sockaddr_in* client, char* buffer);
void logout(struct sockaddr_in* client, char* buffer);
void send_msg(struct sockaddr_in* client, char* buffer);
void broadcast(struct sockaddr_in* client, char* buffer);
void share(struct sockaddr_in* client, char* buffer);
void parse_command(struct sockaddr_in* client, char* buffer);



#endif // SERVER_IMPLEMENTATION_H_INCLUDED
