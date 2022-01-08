#include "client.h"
#include "server.h"
#include "arrayList.h"


int main(int argc, char * argv[])
{
    printf("Som v maine\n");

    if (strcmp(argv[1],"server") == 0) {
        printf("Spustam %s\n",argv[1]);
        startServer(argc, argv);
    } else {
        printf("Spustam %s\n",argv[1]);
        startClient(argc, argv);

    }

    return 0;
}
