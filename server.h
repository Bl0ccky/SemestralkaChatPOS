#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>

#ifndef CHAT_SERVER_H
#define CHAT_SERVER_H

#define MAX_INBOX_SIZE 10
#define MAX_BUFFER_SIZE 512


typedef struct
{
    struct sockaddr_in cl_address;
    int sockfd;
    int id;
    char * name;
    char addRequests[MAX_INBOX_SIZE*MAX_BUFFER_SIZE];
    char removeNotifications[MAX_INBOX_SIZE*MAX_BUFFER_SIZE];
    int numOfRemoveNotifications;
    int numOfAddRequests;
    bool isOnline;
} Client;

int startServer(int argc, char *argv[]);
#endif //CHAT_SERVER_H
