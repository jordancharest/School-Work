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
#define MAX_BUFFER 1024



// GLOBAL ========================================================================================
typedef struct user {
    char userID[21];
    int active;
    int socket;
    int name_len;
    struct sockaddr_in* client;
    char conn_type[4];
} user_t;

user_t* active_users;
extern unsigned int num_active;
extern pthread_t master_thread;
extern pthread_mutex_t user_lock;


// FUNCTION PROTOTYPES ===========================================================================
void login_attempt(int socket, struct sockaddr_in* client, char* buffer, char* conn_type);
void who(int socket, struct sockaddr_in* client, char* buffer, char* conn_type);
void logout(int socket, struct sockaddr_in* client, char* conn_type);
void send_msg(int socket, struct sockaddr_in* client, char* buffer, char* conn_type);
void broadcast(int socket, struct sockaddr_in* client, char* buffer, char* conn_type);
void share(int socket, struct sockaddr_in* client, char* buffer, char* conn_type);
void parse_command(int UDP_socket, struct sockaddr_in* client, char* buffer, char* conn_type);



#endif // SERVER_IMPLEMENTATION_H_INCLUDED
