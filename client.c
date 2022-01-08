#include "client.h"
#include <stdbool.h>

#define MAX_BUFFER_SIZE 1024

int initSocket(int * port, char * host, int * sockfd, struct sockaddr_in * serv_addr, struct hostent * server)
{
    server = gethostbyname(host);
    if (server == NULL)
    {
        fprintf(stderr, "Error, no such host\n");
        return 2;
    }

    //Socket settings
    bzero((char*)serv_addr, sizeof(*serv_addr));
    serv_addr->sin_family = AF_INET;
    bcopy(
            (char*)server->h_addr,
            (char*)&serv_addr->sin_addr.s_addr,
            server->h_length
    );
    serv_addr->sin_port = htons(*port);
    *sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (*sockfd < 0)
    {
        perror("Error creating socket");
        return 3;
    }

    //Conection socket
    if(connect(*sockfd, (struct sockaddr*)serv_addr, sizeof(*serv_addr)) < 0)
    {
        perror("Error connecting to socket");
        return 4;
    }
    printf("Connected\n");

    return *sockfd;

}

int startClient(int argc, char *argv[])
{

    if (argc < 3)
    {
        fprintf(stderr,"usage %s hostname port\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[3]);
    char * host = argv[2];
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent server;
    initSocket(&port, host, &sockfd, &serv_addr, &server);
    char buffer[MAX_BUFFER_SIZE];
    while (true)
    {
        bzero(buffer,MAX_BUFFER_SIZE);
        read(sockfd, buffer, MAX_BUFFER_SIZE);
        printf("%s\n",buffer);
        bzero(buffer,MAX_BUFFER_SIZE);
        fgets(buffer, MAX_BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0;
        printf("Zadal si:%s\n",buffer);

        if(strcmp(buffer, "exit") == 0)
        {
            break;
        }
        write(sockfd, buffer, strlen (buffer)+1);


    }

    printf("Zavrel som socketClienta\n");
    close(sockfd);

    return 0;
}