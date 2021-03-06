#include <sys/stat.h>
#include "server.h"

char *pathToFiles = "/home/labat2/SemestralkaChat";

Client *getClient(arraylist *array, char *name) {

    for (int i = 0; i < array->size; i++) {
        Client *foundClient = arraylist_get(array, i);
        if (strcmp(name, foundClient->name) == 0) {
            return foundClient;
        }
    }
    return NULL;

}

void initListeningSocket(const int *port, int *listenfd, struct sockaddr_in *serv_addr) {

    printf("Zacinam Listening init\n");
    //Socket settings
    bzero((char *) serv_addr, sizeof(*serv_addr));
    *listenfd = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr->sin_family = AF_INET;
    serv_addr->sin_addr.s_addr = INADDR_ANY;
    serv_addr->sin_port = htons(*port);

    //Creating listen socket
    if (listenfd < 0) {
        perror("Error creating listening socket");
    }

    //Bind
    if (bind(*listenfd, (struct sockaddr *) serv_addr, sizeof(*serv_addr)) < 0) {
        perror("Error binding socket address");
    }

    //Listen
    if (listen(*listenfd, 10) < 0) {
        perror("ERROR on listen");
    }
    printf("Koncim Listening init\n");

}

void initConnSocket(const int *listenfd, int *connfd, struct sockaddr_in *cli_addr) {
    printf("Zacinam Conn init\n");
    socklen_t cli_len;

    //Creating conn socket
    cli_len = sizeof(*cli_addr);
    *connfd = accept(*listenfd, (struct sockaddr *) cli_addr, &cli_len);

    if (connfd < 0) {
        perror("ERROR on accept socket");
    }
    printf("Koncim Conn init\n");
}

int countNumOfLines(char *fileName, pthread_mutex_t * mutex)
{
    FILE *file;
    if(mutex)
    {
        pthread_mutex_lock(mutex);
    }

    file = fopen(fileName, "r");
    int size = 0;
    printf("Idem zratat riadky suboru %s\n", fileName);
    char chr = getc(file);

    if (chr != EOF) {
        while (chr != EOF) {

            if (chr == '\n') {
                size++;
            }
            chr = getc(file);
        }
    }

    fclose(file);
    if(mutex)
    {
        pthread_mutex_unlock(mutex);
    }

    printf("Subor %s ma %d riadkov\n", fileName, size);
    return size;
}

void checkCorrectInput(Client *client, char *option1, char *option2, char *result) {
    bool correctInput = false;
    char option[MAX_BUFFER_SIZE];
    char msg[MAX_BUFFER_SIZE];
    while (!correctInput) {
        read(client->sockfd, option, sizeof(option));
        if (strcmp(option, option1) == 0 || strcmp(option, option2) == 0) {
            correctInput = true;
        } else {
            sprintf(msg, "Nespravna volba! Stlacte %s alebo %s!", option1, option2);
            write(client->sockfd, msg, strlen(msg) + 1);
        }
    }
    strcpy(result, option);
}

bool checkExistingUser(Client * client, char *login, char *password) {

    FILE *users;
    char fileName[MAX_BUFFER_SIZE];
    sprintf(fileName, "%s/chatUsers.txt", pathToFiles);
    int numOfLines = countNumOfLines(fileName, client->sharedData->mutexRegisteredUsers);
    pthread_mutex_lock(client->sharedData->mutexRegisteredUsers);

    users = fopen(fileName, "r");
    if (users == NULL) {
        perror("DONE\n");
    }

    char foundLogin[MAX_BUFFER_SIZE] = "";
    char foundPassword[MAX_BUFFER_SIZE] = "";
    if (numOfLines > 0) {
        while (!feof(users)) {
            fscanf(users, "%s\t%s", foundLogin, foundPassword);
            if (strcmp(foundLogin, login) == 0) {
                strcpy(password, foundPassword);
                pthread_mutex_unlock(client->sharedData->mutexRegisteredUsers);
                return true;
            }
        }

    }
    fclose(users);
    pthread_mutex_unlock(client->sharedData->mutexRegisteredUsers);
    return false;

}

void log(Client *client, char *login);

void registration(Client *client, char *login) {
    char *msg;
    char password[MAX_BUFFER_SIZE];
    while (checkExistingUser(client, login, password)) {
        msg = "Zadany login uz existuje, zadaj iny, alebo klikni 2 pre prihlasanie";
        write(client->sockfd, msg, strlen(msg) + 1);
        read(client->sockfd, login, MAX_BUFFER_SIZE);
        if (strcmp(login, "2") == 0) {
            msg = "Zadaj login";
            write(client->sockfd, msg, strlen(msg) + 1);
            read(client->sockfd, login, MAX_BUFFER_SIZE);
            log(client, login);
            return;
        }
    }
    msg = "Zadaj heslo";
    write(client->sockfd, msg, strlen(msg) + 1);
    read(client->sockfd, password, sizeof(password));

    char fileName[MAX_BUFFER_SIZE];
    FILE *users;
    FILE *user;

    sprintf(fileName, "%s/%s", pathToFiles, login);
    mkdir(fileName, 0777);

    sprintf(fileName, "%s/%s/%sRequests.txt", pathToFiles, login, login);
    user = fopen(fileName, "w");
    fclose(user);

    sprintf(fileName, "%s/%s/%sFriends.txt", pathToFiles, login, login);
    user = fopen(fileName, "w");
    fclose(user);

    sprintf(fileName, "%s/%s/%sMessages.txt", pathToFiles, login, login);
    user = fopen(fileName, "w");
    fclose(user);

    sprintf(fileName, "%s/chatUsers.txt", pathToFiles);
    pthread_mutex_lock(client->sharedData->mutexRegisteredUsers);
    users = fopen(fileName, "a");
    fprintf(users, "%s\t%s\n", login, password);
    fclose(users);
    pthread_mutex_unlock(client->sharedData->mutexRegisteredUsers);
    printf("Vytvoril som usera s loginom %s a heslom %s zapisal ho do suboru\n", login, password);
}

void acceptFriendRequest(Client *client, char *recv, char *send) {

    char option[MAX_BUFFER_SIZE];
    char fileName[MAX_BUFFER_SIZE];
    char infoReq[MAX_BUFFER_SIZE];

    printf("%s posiela ziadost %s o priatelstvo\n", send, recv);
    sprintf(infoReq, "%s vam posiela ziadost o priatelstvo, chcete ho prijat? [A|N]", send);
    write(client->sockfd, infoReq, strlen(infoReq) + 1);
    checkCorrectInput(client, "A", "N", option);

    if (strcmp(option, "A") == 0) {
        printf("%s si zvolil moznost A - pridat priatela %s\n", recv, send);
        FILE *friendsFile;
        sprintf(fileName, "%s/%s/%sFriends.txt", pathToFiles, recv, recv);
        pthread_mutex_lock(client->sharedData->mutexFriends);
        friendsFile = fopen(fileName, "a");
        fprintf(friendsFile, "%s\n", send);
        fclose(friendsFile);
        pthread_mutex_unlock(client->sharedData->mutexFriends);
        printf("Priatel %s bol pridany do suboru %s\n", send, fileName);

        sprintf(fileName, "%s/%s/%sFriends.txt", pathToFiles, send, send);
        pthread_mutex_lock(client->sharedData->mutexFriends);
        friendsFile = fopen(fileName, "a");
        fprintf(friendsFile, "%s\n", recv);
        fclose(friendsFile);
        pthread_mutex_unlock(client->sharedData->mutexFriends);
        printf("Priatel %s bol pridany do suboru %s\n", recv, fileName);
    }

}

bool checkAlreadyInFile(pthread_mutex_t * mutex, char *fileName, char *login) {
    FILE *file;
    printf("Idem sa pokusit otvorit %s\n", fileName);
    int numOfLines = countNumOfLines(fileName, mutex);
    pthread_mutex_lock(mutex);
    file = fopen(fileName, "r");
    char foundLogin[MAX_BUFFER_SIZE] = "";
    bool found = false;

    if (numOfLines > 0) {
        for (int i = 0; i < numOfLines; ++i) {
            fscanf(file, "%s", foundLogin);
            printf("Precital som %s\n", foundLogin);
            if (strcmp(foundLogin, login) == 0) {
                printf("Nasiel som %s\n", foundLogin);
                found = true;
                break;
            }

        }
    }
    fclose(file);
    pthread_mutex_unlock(mutex);
    printf("checkhol som subor %s\n", fileName);
    printf("V %s result: %d\n", fileName, found);

    return found;
}

void checkFriendsRequests(Client *client) {
    printf("Idem otvorit friend requesty uzivatela %s\n", client->name);

    char fileName[MAX_BUFFER_SIZE];
    sprintf(fileName, "%s/%s/%sRequests.txt", pathToFiles, client->name, client->name);
    int numOfRequests = countNumOfLines(fileName, client->sharedData->mutexRequests);

    printf("Pocet requestov uzivatela %s je %d\n", client->name, numOfRequests);
    if (numOfRequests > 0) {
        char sender[MAX_BUFFER_SIZE];
        FILE *requestFile;
        pthread_mutex_lock(client->sharedData->mutexRequests);
        requestFile = fopen(fileName, "r");
        sprintf(fileName, "%s/chatUsers.txt", pathToFiles);
        char password[MAX_BUFFER_SIZE];
        while (numOfRequests > 0) {
            fscanf(requestFile, "%s\n", sender);
            if(checkExistingUser(client, sender, password))
            {
                acceptFriendRequest(client, client->name, sender);
            }
            numOfRequests--;
        }

        fclose(requestFile);
        sprintf(fileName, "%s/%s/%sRequests.txt", pathToFiles, client->name, client->name);
        remove(fileName);
        printf("Uzivatel %s si pozrel vsetky jeho requesty a tak sa zmazal subor %s\n", client->name, fileName);
        FILE *newRequest = fopen(fileName, "w");
        fclose(newRequest);
        pthread_mutex_unlock(client->sharedData->mutexRequests);
        printf("Vytvoril som novy subor na requesty uzivatela %s nazvom %s\n", client->name, fileName);
    }

    pthread_mutex_lock(client->mutexAddRequests);
    if(client->numOfAddRequests>0)
    {
        client->numOfAddRequests=0;
    }
    pthread_mutex_unlock(client->mutexAddRequests);
}

bool checkAlreadyLoggedIn(Client *client, char *login) {
    pthread_mutex_lock(client->sharedData->mutexOnlineUsers);
    for (int i = 0; i < arraylist_size(client->sharedData->onlineUsers); ++i) {
        if (getClient(client->sharedData->onlineUsers, login)) {
            pthread_mutex_unlock(client->sharedData->mutexOnlineUsers);
            return true;
        }
    }
    pthread_mutex_unlock(client->sharedData->mutexOnlineUsers);
    return false;
}

void log(Client *client, char *login) {
    char foundPassword[MAX_BUFFER_SIZE];
    char password[MAX_BUFFER_SIZE];
    char *msg;
    while (!checkExistingUser(client, login, foundPassword) || checkAlreadyLoggedIn(client, login)) {
        if (!checkExistingUser(client, login, foundPassword)) {
            printf("Uzivatel %s zadal neregistrovany login\n", client->name);
            msg = "Zadany login nie je zaregistrovany, zadaj registrovany login, alebo klikni 1 pre registruaciu!";
        } else {
            printf("Uzivatel %s zadal login, ktory je uz prihlaseny\n", client->name);
            msg = "Zadany login je uz prihlaseny, zadaj iny login, alebo klikni 1 pre registruaciu!";
        }
        write(client->sockfd, msg, strlen(msg) + 1);
        read(client->sockfd, login, MAX_BUFFER_SIZE);
        if (strcmp(login, "1") == 0) {
            msg = "Zadaj login";
            write(client->sockfd, msg, strlen(msg) + 1);
            read(client->sockfd, login, MAX_BUFFER_SIZE);
            registration(client, login);
            return;
        }
    }

    msg = "Zadaj heslo";
    write(client->sockfd, msg, strlen(msg) + 1);
    read(client->sockfd, password, sizeof(password));
    while (strcmp(foundPassword, password) != 0) {
        printf("Uzivatel %s zadal nespravne heslo\n", client->name);
        msg = "Zadane heslo sa nezhoduje so zadanym loginom, zadaj spravne heslo!";
        write(client->sockfd, msg, strlen(msg) + 1);
        read(client->sockfd, password, sizeof(password));
    }


}

void checkNewRemoval(Client *client) {
    FILE *file;
    char fileName[MAX_BUFFER_SIZE];
    char buffer[MAX_BUFFER_SIZE];
    sprintf(fileName, "%s/%s/%sRemoveFriends.txt", pathToFiles, client->name, client->name);
    bool opened = false;
    pthread_mutex_lock(client->sharedData->mutexRemovedFriend);
    if (file = fopen(fileName, "r")) {
        opened = true;
        char removed[MAX_BUFFER_SIZE];
        strcpy(buffer, "Z priatelov si ta odstranili:\n");
        while (!feof(file)) {
            fscanf(file, "%s", removed);
            printf("Uzivatel %s si odstranil uzivatela %s z priatelov :(\n", removed, client->name);
            strcat(buffer, removed);
            strcat(buffer, "\n");
        }
        fclose(file);
        pthread_mutex_unlock(client->sharedData->mutexRemovedFriend);
        remove(fileName);
        strcat(buffer, "Pre navrat do menu stlac lubovolnu klavesu");
        write(client->sockfd, buffer, strlen(buffer) + 1);
        read(client->sockfd, buffer, sizeof(buffer));
    }
    if(!opened)
    {
        pthread_mutex_unlock(client->sharedData->mutexRemovedFriend);
        pthread_mutex_lock(client->mutexRemoveNotifications);
        if(client->numOfRemoveNotifications>0)
        {
            client->numOfRemoveNotifications=0;
        }
        pthread_mutex_unlock(client->mutexRemoveNotifications);
    }



}

void onlineUserMenu(Client *client);

void regLogin(Client *client) {
    char buffer[MAX_BUFFER_SIZE];
    char *msg = "Zadaj co chces robit:\n1.Registracia\n2.Prihlasenie";
    write(client->sockfd, msg, strlen(msg) + 1);
    checkCorrectInput(client, "1", "2", buffer);
    msg = "Zadaj login";
    write(client->sockfd, msg, strlen(msg) + 1);
    char login[MAX_BUFFER_SIZE];
    read(client->sockfd, login, sizeof(login));

    if (strcmp(buffer, "1") == 0) {
        printf("Uzivatel %s sa ide registrovat\n", login);
        registration(client, login);
    } else {
        printf("Uzivatel %s sa ide prihlasovat\n", login);
        log(client, login);
    }

    strcpy(client->name, login);
    pthread_mutex_lock(client->sharedData->mutexOnlineUsers);
    arraylist_add(client->sharedData->onlineUsers, client);
    pthread_mutex_unlock(client->sharedData->mutexOnlineUsers);

    if (strcmp(buffer, "2") == 0) {
        checkFriendsRequests(client);
        checkNewRemoval(client);
    }
    printf("Klient s loginom %s a socketom %d je pripojeny:)\n", client->name, client->sockfd);
    onlineUserMenu(client);

}



bool checkAlreadyFriendsOrAlreadyInRequests(Client *client, char *friendName) {

    char friendsFile[MAX_BUFFER_SIZE];
    char requestsFile1[MAX_BUFFER_SIZE];
    char requestsFile2[MAX_BUFFER_SIZE];
    sprintf(friendsFile, "%s/%s/%sFriends.txt", pathToFiles, friendName, friendName);
    sprintf(requestsFile1, "%s/%s/%sRequests.txt", pathToFiles, friendName, friendName);
    sprintf(requestsFile2, "%s/%s/%sRequests.txt", pathToFiles, client->name, client->name);


    printf("Idem skontrolovat subor %s ci sa v nom nachadza uzivatel %s\n", requestsFile1, client->name);
    printf("Idem skontrolovat subor %s ci sa v nom nachadza uzivatel %s\n", requestsFile2, friendName);
    printf("Idem skontrolovat subor %s ci sa v nom nachadza uzivatel %s\n", friendsFile, client->name);


    return checkAlreadyInFile(client->sharedData->mutexFriends, friendsFile, client->name) ||
           checkAlreadyInFile(client->sharedData->mutexRequests, requestsFile1, client->name) ||
           checkAlreadyInFile(client->sharedData->mutexRequests, requestsFile2, friendName);
}

bool isOnline(Client *client, char *name, int *index) {
    pthread_mutex_lock(client->sharedData->mutexOnlineUsers);
    for (int i = 0; i < arraylist_size(client->sharedData->onlineUsers); ++i) {
        Client *foundClient = arraylist_get(client->sharedData->onlineUsers, i);
        if (strcmp(foundClient->name, name) == 0) {
            pthread_mutex_unlock(client->sharedData->mutexOnlineUsers);
            *index = i;
            return true;
        }
    }
    pthread_mutex_unlock(client->sharedData->mutexOnlineUsers);
    return false;
}


void logout(Client *client) {

    pthread_mutex_lock(client->mutexRemoveNotifications);
    bool hasNotifications=client->numOfRemoveNotifications > 0;
    printf("Uzivatela %s si %d ludi odstranilo z priatelov\n", client->name, client->numOfRemoveNotifications);
    pthread_mutex_unlock(client->mutexRemoveNotifications);
    if (hasNotifications) {
        checkNewRemoval(client);
    }

    pthread_mutex_lock(client->mutexRemoveNotifications);
    hasNotifications=client->numOfAddRequests > 0;
    printf("Uzivatel %s ma %d ziadosti o priatelstvo\n", client->name, client->numOfAddRequests);
    pthread_mutex_unlock(client->mutexRemoveNotifications);
    if (hasNotifications) {
        checkFriendsRequests(client);
    }

    int index = -1;
    isOnline(client, client->name, &index);
    pthread_mutex_lock(client->sharedData->mutexOnlineUsers);
    arraylist_remove(client->sharedData->onlineUsers, index);
    pthread_mutex_unlock(client->sharedData->mutexOnlineUsers);
    printf("Uzivatel s loginom %s a socketom %d sa odhlasil\n", client->name, client->sockfd);

    char * msg="exit";
    write(client->sockfd,msg,strlen(msg)+1);
    close(client->sockfd);
    printf("Zavrel som socket klienta s cislom %d\n",client->sockfd);
    free(client);

}

void removeLineFromFile(char *remLogin, char *fileName, bool removeFriend,pthread_mutex_t * mutex) {
    FILE *users;
    FILE *newUsers;
    printf("Idem Vymazat riadok zo suboru %s\n", fileName);

    int numOfRegistered = countNumOfLines(fileName, mutex);
    if(mutex)
    {
        pthread_mutex_lock(mutex);
    }

    users = fopen(fileName, "r");
    char tmpFileName[MAX_BUFFER_SIZE];
    sprintf(tmpFileName, "%s/tmp.txt", pathToFiles);
    newUsers = fopen(tmpFileName, "w");
    char foundLogin[MAX_BUFFER_SIZE];
    char foundPassword[MAX_BUFFER_SIZE];

    for (int i = 0; i < numOfRegistered; i++) {
        if (removeFriend) {
            fscanf(users, "%s\n", foundLogin);
        } else {
            fscanf(users, "%s\t%s\n", foundLogin, foundPassword);
        }
        if (strcmp(remLogin, foundLogin) != 0) {
            if (removeFriend) {
                fprintf(newUsers, "%s\n", foundLogin);
                printf("Odstranil som %s\n", foundLogin);
            } else
                fprintf(newUsers, "%s\t%s\n", foundLogin, foundPassword);
        }

    }
    fclose(users);
    fclose(newUsers);

    if(mutex)
    {
        pthread_mutex_unlock(mutex);
    }
    remove(fileName);
    rename(tmpFileName, fileName);
}

void removeUserFromFriends(char *userName, char *removingUserName,pthread_mutex_t * mutex) {

    char fileName[MAX_BUFFER_SIZE];
    sprintf(fileName, "%s/%s/%sFriends.txt", pathToFiles, userName, userName);
    printf("Idem zmazat uzivatela %s zo suboru %s", removingUserName, fileName);
    removeLineFromFile(removingUserName, fileName, true,mutex);
    printf("Zmazal som uzivatela %s zo suboru %s", removingUserName, fileName);

}


void removeAccount(Client *client) {

    printf("Idem odstranit uzivatela %s\n", client->name);
    char fileName[MAX_BUFFER_SIZE];
    sprintf(fileName, "%s/chatUsers.txt", pathToFiles);
    removeLineFromFile(client->name, fileName, false,client->sharedData->mutexRegisteredUsers);
    sprintf(fileName, "%s/%s/%sFriends.txt", pathToFiles, client->name, client->name);
    int numOfFriends = countNumOfLines(fileName, client->sharedData->mutexFriends);
    pthread_mutex_lock(client->sharedData->mutexFriends);
    FILE *file = fopen(fileName, "r");

    char friend[MAX_BUFFER_SIZE];
    for (int i = 0; i < numOfFriends; ++i) {
        fscanf(file, "%s", friend);
        pthread_mutex_unlock(client->sharedData->mutexFriends);
        removeUserFromFriends(friend, client->name,client->sharedData->mutexFriends);
        pthread_mutex_lock(client->sharedData->mutexFriends);
    }
    fclose(file);
    pthread_mutex_unlock(client->sharedData->mutexFriends);
    remove(fileName);

    int index = -1;
    isOnline(client, client->name, &index);
    pthread_mutex_lock(client->sharedData->mutexOnlineUsers);
    arraylist_remove(client->sharedData->onlineUsers, index);
    pthread_mutex_unlock(client->sharedData->mutexOnlineUsers);

    sprintf(fileName, "%s/%s", pathToFiles, client->name);
    char cmd[MAX_BUFFER_SIZE];
    sprintf(cmd, "rm -rf %s", fileName);
    system(cmd);
    printf("Odstranil som uzivatela %s a vsetky jeho subory spolu s priecinkom\n", client->name);

    char * msg="exit";
    write(client->sockfd,msg,strlen(msg)+1);
    close(client->sockfd);
    printf("Zavrel som socket klienta s cislom %d\n",client->sockfd);
    free(client);


}

void printUsers(Client *client, char *fileName, char *contact, int funOption, pthread_mutex_t * mutex);


void addFriend(Client *client) {
    printf("Uzivatel %s si ide pridat noveho priatela\n", client->name);
    char choose[MAX_BUFFER_SIZE];
    char msg[MAX_BUFFER_SIZE];
    char fileName[MAX_BUFFER_SIZE];
    sprintf(fileName, "%s/chatUsers.txt", pathToFiles);

    int numOfLines = countNumOfLines(fileName, client->sharedData->mutexRegisteredUsers);

    if (numOfLines == 1) {
        printf("Uzivatel %s nema na vyber ziadneho noveho priatela\n", client->name);
        sprintf(msg, "Ziadny uzivatelia na pridanie, pre navrat do menu stlac lubovolnu klavesu");
        write(client->sockfd, msg, strlen(msg) + 1);
        read(client->sockfd, choose, sizeof(choose));
        onlineUserMenu(client);
        return;
    }

    printUsers(client, fileName, choose, 3, client->sharedData->mutexRegisteredUsers);
    if(strcmp(choose, "1") != 0)
    {
        int index = -1;
        bool print=true;
        if (isOnline(client, choose, &index)) {
            print=false;
            pthread_mutex_lock(client->sharedData->mutexOnlineUsers);
            Client *choosenUser = arraylist_get(client->sharedData->onlineUsers, index);
            pthread_mutex_unlock(client->sharedData->mutexOnlineUsers);
            printf("Pocet novych ziadosti o priatelstvo uzivatela %s je  %d\n", client->name, client->numOfAddRequests);
            printf("Idem skontrolovat ci uzivatel %s uz ziadal uzivatela %s o priatelstvo\n", client->name,choosenUser->name);
            printf("Uzivatel %s este neziadal uzivatela %s o priatelstvo", client->name, choosenUser->name);

            char requestFileName[MAX_BUFFER_SIZE];
            sprintf(requestFileName,"%s/%s/%sRequests.txt",pathToFiles,client->name,client->name);
            if(!checkAlreadyInFile(client->sharedData->mutexRequests,requestFileName,choosenUser->name))
            {
                print=true;
                pthread_mutex_lock(client->mutexAddRequests);
                printf("Uzivatel %s si pridal do ziadosti o priatelstvo uzivatela %s\n", client->name,choosenUser->name);
                (choosenUser->numOfAddRequests)++;
                pthread_mutex_unlock(client->mutexAddRequests);

            }
            else
            {
                checkFriendsRequests(client);
                onlineUserMenu(client);
            }
        }
        if(print)
        {
            char sender[MAX_BUFFER_SIZE];
            strcpy(sender, client->name);
            FILE *choosenFile;
            sprintf(fileName, "%s/%s/%sRequests.txt", pathToFiles, choose, choose);
            pthread_mutex_lock(client->sharedData->mutexRequests);
            choosenFile = fopen(fileName, "a");
            fprintf(choosenFile, "%s\n", sender);
            fclose(choosenFile);
            pthread_mutex_unlock(client->sharedData->mutexRequests);
            printf("Uzivatel %s poslal uzivatelovi %s ziadost o priatelstvo\n", client->name, choose);
            onlineUserMenu(client);
        }
    }



}

void printUsers(Client *client, char *fileName, char *contact, int funOption, pthread_mutex_t * mutex) {
    FILE *file;
    char message[MAX_BUFFER_SIZE];
    char buffer[MAX_BUFFER_SIZE];
    int numOfFriends = countNumOfLines(fileName, mutex);

    sprintf(message, "Vyber si uzivatela, alebo stlac 1 pre navrat do menu");
    if(mutex)
    {
        pthread_mutex_lock(mutex);
    }
    file = fopen(fileName, "r");
    char users[MAX_BUFFER_SIZE * numOfFriends];
    char user[MAX_BUFFER_SIZE];
    bool print;
    for (int i = 0; i < numOfFriends; ++i) {
        print = false;
        if (funOption == 3)
        {
            fscanf(file, "%s %s", user, buffer);
            printf("Porovnavam %s a %s\n", user, client->name);

            pthread_mutex_unlock(mutex);
            char requestFile[MAX_BUFFER_SIZE];
            sprintf(requestFile,"%s/%s/%sRequests.txt",pathToFiles,user,user);
            if (strcmp(user, client->name) != 0 && !checkAlreadyFriendsOrAlreadyInRequests(client, user))
            {
                printf("Uzivatel %s nema este uzivatela %s v priateloch\n", user, client->name);
                print = true;
            }
            pthread_mutex_lock(mutex);
        }
        else
        {
            fscanf(file, "%s\n", user);
            print = true;
        }
        if (print) {
            strcat(message, "\n");
            strcat(message, user);

            printf("Precital som %s\n", user);
            strcpy(users + i * MAX_BUFFER_SIZE, user);
            printf("Target uzivatela %s je %s\n", client->name, users + i * MAX_BUFFER_SIZE);

        }

    }
    fclose(file);
    if(mutex)
    {
        pthread_mutex_unlock(mutex);
    }
    write(client->sockfd, message, strlen(message) + 1);
    strcat(message,"\nZadaneho uzivatela nie je mozne zvolit, vyber si niekoho z vyssie zadanych alebo klikni 1 pre navrat do menu.");
    bool done = false;
    while (!done) {
        read(client->sockfd, buffer, sizeof(buffer));
        if (strcmp(buffer, "1") == 0) {
            onlineUserMenu(client);
            return;
        }
        printf("User si choosol %s\n", buffer);
        for (int i = 0; i < numOfFriends; i++) {
            printf("Kontrolujem %s\n", users + (i * MAX_BUFFER_SIZE));
            if (strcmp(users + (i * MAX_BUFFER_SIZE), buffer) == 0) {
                printf("Input je ok:)\n");
                done = true;
                break;
            }
        }
        if (!done) {
            write(client->sockfd, message, strlen(message) + 1);
        }
    }
    strcpy(contact, buffer);

}


void removeFriend(Client *client) {

    printf("Uzivatel %s si ide odstranit nejakeho priatela zo zoznamu priatelov\n", client->name);
    char fileName[MAX_BUFFER_SIZE];
    sprintf(fileName, "%s/%s/%sFriends.txt", pathToFiles, client->name, client->name);
    char friend[MAX_BUFFER_SIZE];

    int numOfLines = countNumOfLines(fileName, client->sharedData->mutexFriends);
    if (numOfLines == 0) {
        printf("Uzivatel %s nema ziadnych kamaratov, nema si koho odstranit\n", client->name);
        sprintf(friend, "Ziadny priatelia na odstranenie, pre navrat do menu stlac lubovolnu klavesu");
        write(client->sockfd, friend, strlen(friend) + 1);
        read(client->sockfd, fileName, sizeof(fileName));
        onlineUserMenu(client);
        return;
    }

    printUsers(client, fileName, friend, 2, client->sharedData->mutexFriends);
    printf("Uzivatel %s si vybral ze chce odstranit: %s\n", client->name, friend);
    pthread_mutex_lock(client->sharedData->mutexOnlineUsers);
    Client *removingUser = getClient(client->sharedData->onlineUsers, friend);
    pthread_mutex_unlock(client->sharedData->mutexOnlineUsers);
    FILE *friendsFile;
    if (removingUser) {
        pthread_mutex_lock(removingUser->mutexRemoveNotifications);
        printf("Uzivatel %s je online a tak sa mu notifikacia o tom ze si ho niekto odstranil z priatelov zapisala do pola\n", removingUser->name);
        removingUser->numOfRemoveNotifications++;
        pthread_mutex_unlock(removingUser->mutexRemoveNotifications);
    }

    sprintf(fileName, "%s/%s/%sFriends.txt", pathToFiles, client->name, client->name);
    if(checkAlreadyInFile(client->sharedData->mutexFriends,fileName,friend))
    {
        sprintf(fileName, "%s/%s/%sRemoveFriends.txt", pathToFiles, friend, friend);
        pthread_mutex_lock(client->sharedData->mutexRemovedFriend);
        friendsFile = fopen(fileName, "a");
        fprintf(friendsFile, "%s", client->name);
        fclose(friendsFile);
        pthread_mutex_unlock(client->sharedData->mutexRemovedFriend);
        printf("Uzivatel %s je offline a tak sa mu uzivatel, ktory si ho odstranil zo zoznamu priatelov zapisal do suboru %s\n", friend, fileName);

        removeUserFromFriends(client->name, friend,client->sharedData->mutexFriends);
        removeUserFromFriends(friend, client->name,client->sharedData->mutexFriends);
        printf("Uzivatel %s si odstranil uzivatela %s z priatelov\n", client->name, friend);
    }
    else
    {
        checkNewRemoval(client);
    }

    onlineUserMenu(client);
}

void readAndEncryptMsg(Client *client, char *encryptedMsg) {

    char msg[MAX_BUFFER_SIZE];
    char buffer[MAX_BUFFER_SIZE];
    sprintf(msg, "Zadaj spravu, ktoru chces poslat:");
    write(client->sockfd, msg, strlen(msg) + 1);
    read(client->sockfd, msg, MAX_BUFFER_SIZE);
    printf("Uzivatel %s posiela spravu %s uzivatelovi do suboru\n", client->name, msg);

    sprintf(buffer, "%s: %s", client->name, msg);

    for (int i = 0; (i < MAX_BUFFER_SIZE && buffer[i] != '\0'); i++) {
        buffer[i] = buffer[i] + 3;
    }
    strcpy(encryptedMsg, buffer);
}

void sendMsg(Client *client) {

    printf("Uzivatel %s ide poslat spravu niekomu zo zoznamu priatelov\n", client->name);
    FILE *file;
    char receiver[MAX_BUFFER_SIZE];
    char buffer[MAX_BUFFER_SIZE];
    char msg[MAX_BUFFER_SIZE];
    sprintf(buffer, "%s/%s/%sFriends.txt", pathToFiles, client->name, client->name);

    int numOfLines = countNumOfLines(buffer, client->sharedData->mutexFriends);
    if (numOfLines == 0) {
        printf("Uzivatel %s nema komu napisat, najprv si musi niekoho pridat\n", client->name);
        sprintf(msg,"Nemas komu napisat, najprv si niekoho pridaj:)\nVyber si co chces robit\n1.Pridat priatela\n2.Navrat do menu");
        write(client->sockfd, msg, strlen(msg) + 1);
        read(client->sockfd, buffer, sizeof(buffer));
        if (strcmp(buffer, "1") == 0)
        {
            addFriend(client);
        }
        else
        {
            onlineUserMenu(client);
        }

    }

    printUsers(client, buffer, receiver, 1, client->sharedData->mutexFriends);

    printf("Uzivatel %s si zvolil ze chce spravu poslat uzivatelovi %s\n", client->name, receiver);
    sprintf(buffer, "%s/%s/%sMessages.txt", pathToFiles, receiver, receiver);
    char readMsg[MAX_BUFFER_SIZE];
    readAndEncryptMsg(client, readMsg);
    pthread_mutex_lock(client->sharedData->mutexMessages);
    file = fopen(buffer, "a");
    fprintf(file, "%s\n", readMsg);
    fclose(file);
    pthread_mutex_unlock(client->sharedData->mutexMessages);
    onlineUserMenu(client);
}

void checkMsgInbox(Client *client) {
    printf("Uzivatel %s si ide pozriet neprecitane spravy\n", client->name);
    FILE *file;
    char buffer[MAX_BUFFER_SIZE];
    sprintf(buffer, "%s/%s/%sMessages.txt", pathToFiles, client->name, client->name);
    int numOfMsgs = countNumOfLines(buffer, client->sharedData->mutexMessages);
    if (numOfMsgs == 0) {
        printf("Uzivatel %s nema ziadne neprecitane spravy\n", client->name);
        sprintf(buffer, "Nemas ziadne neprecitane spravy, pre navrat do menu stlac lubovolnu klavesu");
        write(client->sockfd, buffer, strlen(buffer) + 1);
        read(client->sockfd, buffer, sizeof(buffer));
        onlineUserMenu(client);
    }

    pthread_mutex_lock(client->sharedData->mutexMessages);
    file = fopen(buffer, "r");
    char msg[MAX_BUFFER_SIZE * numOfMsgs];
    bzero(msg, MAX_BUFFER_SIZE);
    for (int i = 0; i < numOfMsgs; ++i) {
        fscanf(file, "%s", buffer);
        for (int j = 0; (j < MAX_BUFFER_SIZE && buffer[j] != '\0'); j++) {
            buffer[j] = buffer[j] - 3; //the key for encryption is 3 that is added to ASCII value
        }
        strcat(msg, buffer);
        strcat(msg, "\n");
    }
    printf("Vsetky neprecitane spravy uzivatela %s boli desifrovane a vypisane na obrazovku\n", client->name);
    fclose(file);
    pthread_mutex_unlock(client->sharedData->mutexMessages);

    strcat(msg, "Pre navrat do menu stlac lubovolnu klaves");
    write(client->sockfd, msg, strlen(msg) + 1);
    read(client->sockfd, buffer, sizeof(buffer));
    sprintf(buffer, "%s/%s/%sMessages.txt", pathToFiles, client->name, client->name);
    pthread_mutex_lock(client->sharedData->mutexMessages);
    file = fopen(buffer, "w");
    fclose(file);
    pthread_mutex_unlock(client->sharedData->mutexMessages);
    printf("Subor neprecitanych sprav uzivatela %s bol premazany\n", client->name);
    onlineUserMenu(client);
}


void getFileName(char *fileName, char *result) {
    char *savePtr = NULL;
    char *dump;
    for (dump = strtok_r(fileName, "/", &savePtr); dump != NULL; dump = strtok_r(NULL, "/", &savePtr)) {
        strcpy(result, dump);
        printf("token:%s\n", dump);
    }
    printf("Nazov suboru%s\n", result);

}

void copyFileContent(char *originalFilePath, char *copyFilePath, pthread_mutex_t * mutex) {


    int numOfLines = countNumOfLines(originalFilePath, mutex);
    printf("Subor z ktoreho kopcim je: %s\n", originalFilePath);
    char buffer[MAX_BUFFER_SIZE];
    pthread_mutex_lock(mutex);
    FILE *file = fopen(originalFilePath, "r");
    FILE *newFile = fopen(copyFilePath, "w");
    for (int i = 0; i < numOfLines; i++) {
        fscanf(file, "%[^\n]\n", buffer);
        printf("Data from the file:%s\n", buffer);
        fprintf(newFile, "%s\n", buffer);
        printf("Zapisal som do suboru %s\n", buffer);
    }
    fclose(file);
    fclose(newFile);
    pthread_mutex_unlock(mutex);
}

void checkFile(Client *client, char *originalFile) {
    FILE *file;
    file = fopen(originalFile, "r");
    char msg[MAX_BUFFER_SIZE];
    char buffer[MAX_BUFFER_SIZE];
    while (file == NULL) {
        strcpy(msg, "Nepodarilo sa otvorit subor, vyber si ako chces pokracovat\n1.Navrat do menu\n2.Poslat iny subor");
        write(client->sockfd, msg, strlen(msg) + 1);
        read(client->sockfd, buffer, sizeof(buffer));
        if (strcmp(buffer, "1") == 0) {
            onlineUserMenu(client);
            return;
        } else {
            file = fopen(originalFile, "r");
        }

    }
    fclose(file);
}


void sendFile(Client *client) {

    printf("Uzivatel %s ide poslat niekomu subor\n", client->name);
    char msg[MAX_BUFFER_SIZE] = "Zadaj cestu k suboru a nazov suboru, ktory chces poslat:";
    char buffer[MAX_BUFFER_SIZE];
    write(client->sockfd, msg, strlen(msg) + 1);
    read(client->sockfd, buffer, sizeof(buffer));
    printf("Uzivatel %s si vybral subor a cesta k nemu je %s\n", client->name, buffer);

    char sendFileName[MAX_BUFFER_SIZE];
    strcpy(sendFileName, buffer);
    char friendsFileName[MAX_BUFFER_SIZE];
    sprintf(friendsFileName,"%s/%s/%sFriends.txt",pathToFiles,client->name,client->name);

    printUsers(client,friendsFileName,buffer,5,client->sharedData->mutexFriends);

    printf("Uzivatel %s si vybral ze chce poslat subor uzivatelovi %s\n", client->name, buffer);
    checkFile(client, sendFileName);
    char name[MAX_BUFFER_SIZE];
    char originalFileName[MAX_BUFFER_SIZE];
    strcpy(originalFileName, sendFileName);
    getFileName(sendFileName, name);
    char pathToNewFile[MAX_BUFFER_SIZE];
    sprintf(pathToNewFile, "%s/%s/%s", pathToFiles, buffer, name);
    copyFileContent(originalFileName, pathToNewFile, client->sharedData->mutexSendFile);
    printf("Uzivatel %s uspesne poslal subor s nazvom %s uzivatelovi %s, ktory sa nachadza v jeho priecinku\n",client->name, name, buffer);
    onlineUserMenu(client);

}

void makeGroup(Client *client) {

    printf("Uzivatel %s ide vytvorit skupinu, ktorej posle spravu alebo subor\n", client->name);
    char *msg;
    char fileName[MAX_BUFFER_SIZE];
    char tmpFileName[MAX_BUFFER_SIZE];
    sprintf(fileName, "%s/%s/%sFriends.txt", pathToFiles, client->name, client->name);
    sprintf(tmpFileName, "%s/%s/%stmp.txt", pathToFiles, client->name, client->name);
    copyFileContent(fileName, tmpFileName, client->sharedData->mutexFriends);
    int maxMembers = countNumOfLines(tmpFileName, NULL);
    char members[maxMembers * MAX_BUFFER_SIZE];
    int numOfMembers = 0;
    char buffer[MAX_BUFFER_SIZE];

    do {
        msg = "Vyber si co chces robit:\nPridat clena do skupiny(P)\nSkoncit zadavanie(S)";
        write(client->sockfd, msg, strlen(msg) + 1);
        checkCorrectInput(client, "S", "P", buffer);

        if (strcmp(buffer, "P") == 0) {
            printf("Uzivatel si vybral pridat dalsieho uzivatela do skupiny\n");
            char member[MAX_BUFFER_SIZE];
            printUsers(client, tmpFileName, member, 4, NULL);
            if (strcmp(member, "1") != 0) {
                printf("Do skupiny bol pridany uzivatel %s\n", member);
                strcpy(members + numOfMembers * MAX_BUFFER_SIZE, member);
                removeLineFromFile(member, tmpFileName, true,NULL);
                numOfMembers++;
            }
        }

    } while (strcmp(buffer, "S") != 0 && numOfMembers < maxMembers);

    remove(tmpFileName);

    printf("Uzivatel %s si ide vybrat ci chce poslat vsetkym clenom skupiny spravu alebo subor\n", client->name);

    msg = "Vyber si co chces poslat:\n1.Spravu\n2.Subor";
    write(client->sockfd, msg, strlen(msg) + 1);
    checkCorrectInput(client, "1", "2", buffer);

    if (strcmp(buffer, "1") == 0) {
        char message[MAX_BUFFER_SIZE];
        readAndEncryptMsg(client, message);
        for (int i = 0; i < numOfMembers; ++i) {
            sprintf(fileName, "%s/%s/%sMessages.txt", pathToFiles, members + i * MAX_BUFFER_SIZE, members + i * MAX_BUFFER_SIZE);
            pthread_mutex_lock(client->sharedData->mutexMessages);
            FILE *file = fopen(fileName, "a");
            fprintf(file, "%s\n", message);
            fclose(file);
            pthread_mutex_unlock(client->sharedData->mutexMessages);
            printf("Sprava bola zasifrovana a zapisana do suboru sprav uzivatela %s\n", members + i * MAX_BUFFER_SIZE);
        }
    } else {
        printf("Uzivatel %s si zvolil ze chce poslat vsetkym clenom skupiny subor\n", client->name);
        msg = "Zadaj cestu k suboru a nazov suboru, ktory chces polat skupine:";
        write(client->sockfd, msg, strlen(msg) + 1);
        read(client->sockfd, buffer, sizeof(buffer));

        char orifinalFile[MAX_BUFFER_SIZE];
        strcpy(orifinalFile, buffer);
        checkFile(client, buffer);
        char name[MAX_BUFFER_SIZE];
        getFileName(buffer, name);
        char pathToNewFile[MAX_BUFFER_SIZE];

        for (int i = 0; i < numOfMembers; ++i) {
            printf("Uzivatel %s posiela subor s nazvom %s uzivatelovi %s\n", client->name, name, members + MAX_BUFFER_SIZE * i);
            sprintf(pathToNewFile, "%s/%s/%s", pathToFiles, members + MAX_BUFFER_SIZE * i, name);
            copyFileContent(orifinalFile, pathToNewFile, client->sharedData->mutexSendFile);
        }
    }
    onlineUserMenu(client);
}


void onlineUserMenu(Client *client) {

    pthread_mutex_lock(client->mutexRemoveNotifications);
    bool hasNotifications=client->numOfRemoveNotifications > 0;
    printf("Uzivatela %s si %d ludi odstranilo z priatelov\n", client->name, client->numOfRemoveNotifications);
    pthread_mutex_unlock(client->mutexRemoveNotifications);
    if (hasNotifications) {
        checkNewRemoval(client);
    }

    pthread_mutex_lock(client->mutexRemoveNotifications);
    hasNotifications=client->numOfAddRequests > 0;
    printf("Uzivatel %s ma %d ziadosti o priatelstvo\n", client->name, client->numOfAddRequests);
    pthread_mutex_unlock(client->mutexRemoveNotifications);
    if (hasNotifications) {
        checkFriendsRequests(client);
    }

    char option[MAX_BUFFER_SIZE];
    char *msg = "Vyber si co chces robit:\n1.Odhlasit sa\n2.Zrusit ucet\n3.Pridat priatela\n4.Odstranit priatela\n5.Poslat spravu\n6.Zobrazit spravy\n7.Poslat subor\n8.Vytvorit skupinovu konverzaciu";
    write(client->sockfd, msg, strlen(msg) + 1);
    read(client->sockfd, option, sizeof(option));
    if (strcmp(option, "1") == 0) {
        logout(client);
    } else if (strcmp(option, "2") == 0) {
        removeAccount(client);
    } else if (strcmp(option, "3") == 0) {
        addFriend(client);
    } else if (strcmp(option, "4") == 0) {
        removeFriend(client);
    } else if (strcmp(option, "5") == 0) {
        sendMsg(client);
    } else if (strcmp(option, "6") == 0) {
        checkMsgInbox(client);
    } else if (strcmp(option, "7") == 0) {
        sendFile(client);
    } else if (strcmp(option, "8") == 0) {
        makeGroup(client);
    } else {
        onlineUserMenu(client);
    }
}

void *handleClient(void *pdata) {
    Client *dataC = pdata;
    regLogin(dataC);
}

void *serverDone(void *pdata)
{
    ServerEnd * dataS = pdata;
    while (true)
    {
        char buffer[MAX_BUFFER_SIZE];
        bzero(buffer, MAX_BUFFER_SIZE);
        fgets(buffer, MAX_BUFFER_SIZE, stdin);
        printf("precital som %s\n", buffer);
        buffer[strcspn(buffer, "\n")] = 0;
        if(strcmp(buffer, "exit") == 0)
        {
            dataS->done = true;
            break;
        }

    }
    printf("Skoncil som chod servera %d\n", dataS->done);

}



int startServer(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "usage %s port\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[2]);
    int listenfd, connfd;
    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;

    arraylist * onlineUsers;
    onlineUsers = arraylist_create();
    pthread_mutex_t mutexOnlineUsers;
    pthread_mutex_init(&mutexOnlineUsers,NULL);
    pthread_mutex_t mutexRegisteredUsers;
    pthread_mutex_init(&mutexRegisteredUsers,NULL);
    pthread_mutex_t mutexRequests;
    pthread_mutex_init(&mutexRequests,NULL);
    pthread_mutex_t mutexFriends;
    pthread_mutex_init(&mutexFriends,NULL);
    pthread_mutex_t mutexMessages;
    pthread_mutex_init(&mutexMessages,NULL);
    pthread_mutex_t mutexRemovedFriend;
    pthread_mutex_init(&mutexRemovedFriend,NULL);
    pthread_mutex_t mutexSendFile;
    pthread_mutex_init(&mutexSendFile,NULL);
    pthread_mutex_t mutexAddRequests;
    pthread_mutex_init(&mutexAddRequests,NULL);
    pthread_mutex_t mutexRemoveNotifications;
    pthread_mutex_init(&mutexRemoveNotifications,NULL);


    S_DATA shared= {onlineUsers,&mutexOnlineUsers,&mutexRegisteredUsers,&mutexRequests,&mutexFriends,&mutexMessages,&mutexRemovedFriend, &mutexSendFile};
    bool done = false;
    initListeningSocket(&port, &listenfd, &serv_addr);
    ServerEnd sEnd = {done};
    int id = 0;
    pthread_t client, serverEnd;
    pthread_create(&serverEnd, NULL, serverDone, &sEnd);

    pthread_detach(serverEnd);

    while (!sEnd.done) {
        initConnSocket(&listenfd, &connfd, &cli_addr);
        printf("Joinul som sa s clientom\n");
        if(sEnd.done)
        {
            char * msg="exit";
            write(connfd, msg, strlen(msg)+1);
        }
        Client *newClient = (Client *) malloc(sizeof(Client));
        newClient->sockfd = connfd;
        newClient->cl_address = cli_addr;
        newClient->numOfAddRequests = 0;
        newClient->numOfRemoveNotifications = 0;
        newClient->mutexAddRequests = &mutexAddRequests;
        newClient->mutexRemoveNotifications = &mutexRemoveNotifications;
        newClient->sharedData = &shared;
        pthread_create(&client, NULL, handleClient, newClient);
        pthread_detach(client);
        printf("server je %d\n", sEnd.done);
    }


    for (int i = 0; i < arraylist_size(onlineUsers); ++i)
    {
        Client * foundClient = arraylist_get(onlineUsers, i);
        free(foundClient);
    }


    close(listenfd);
    arraylist_destroy(onlineUsers);
    pthread_mutex_destroy(&mutexOnlineUsers);
    pthread_mutex_destroy(&mutexRegisteredUsers);
    pthread_mutex_destroy(&mutexRequests);
    pthread_mutex_destroy(&mutexFriends);
    pthread_mutex_destroy(&mutexMessages);
    pthread_mutex_destroy(&mutexRemovedFriend);
    pthread_mutex_destroy(&mutexSendFile);
    pthread_mutex_destroy(&mutexAddRequests);
    pthread_mutex_destroy(&mutexRemoveNotifications);

    printf("Zavrel som listen\n");
    return 0;


}

