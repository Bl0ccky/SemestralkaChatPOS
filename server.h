#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include "arrayList.h"

#ifndef CHAT_SERVER_H
#define CHAT_SERVER_H

#define MAX_INBOX_SIZE 10
#define MAX_BUFFER_SIZE 512
#define MAX_NAME_SIZE 256

typedef struct sharedData{
    arraylist * onlineUsers;
    pthread_mutex_t * mutexOnlineUsers;
    pthread_mutex_t * mutexRegisteredUsers;
    pthread_mutex_t * mutexRequests;
    pthread_mutex_t * mutexFriends;
    pthread_mutex_t * mutexMessages;
    pthread_mutex_t * mutexRemovedFriend;
    pthread_mutex_t * mutexSendFile;

}S_DATA;

typedef struct
{
    struct sockaddr_in cl_address;
    int sockfd;
    int id;
    char name[MAX_NAME_SIZE];
    char addRequests[MAX_INBOX_SIZE*MAX_BUFFER_SIZE];
    char removeNotifications[MAX_INBOX_SIZE*MAX_BUFFER_SIZE];
    int numOfRemoveNotifications;
    int numOfAddRequests;
    pthread_mutex_t * mutexAddRequests;
    pthread_mutex_t * mutexRemoveNotifications;
    S_DATA * sharedData;
} Client;

typedef struct
{
    bool done;
} ServerEnd;


int startServer(int argc, char *argv[]);
#endif //CHAT_SERVER_H
