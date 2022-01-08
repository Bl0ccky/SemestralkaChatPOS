#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef CHAT_CLIENT_H
#define CHAT_CLIENT_H

int startClient(int argc, char *argv[]);
#endif //CHAT_CLIENT_H
