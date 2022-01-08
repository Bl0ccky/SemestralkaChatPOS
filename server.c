#include <sys/stat.h>
#include "server.h"
#include "arrayList.h"
#include "arrayList.h"


ArrayList onlineUsers;
int numOfloggedIn = 0;
char * pathToFiles="/home/sinalova4/SemestralkaChat";


pthread_mutex_t clientMutex;

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
    if (listen(*listenfd, 5) < 0) {
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
    numOfloggedIn++;
}

int countNumOfLines(char *fileName) {
    FILE *file;
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
    printf("Subor %s ma %d riadkov\n", fileName, size);
    return size;
}

void checkCorrectInput(Client * client, char * option1,char * option2, char * result)
{
    bool correctInput =false;
    char option[MAX_BUFFER_SIZE];
    char msg[MAX_BUFFER_SIZE];
    while (!correctInput) {
        read(client->sockfd, option, sizeof(option));
        if (strcmp(option, option1) == 0 || strcmp(option, option2) == 0) {
            correctInput = true;
        } else {
            sprintf(msg,"Nespravna volba! Stlacte %s alebo %s!",option1,option2);
            write(client->sockfd, msg, strlen(msg) + 1);
        }
    }
    strcpy(result,option);
}

bool checkExistingUser(char *login, char *password) {
    FILE *users;
    char fileName[MAX_BUFFER_SIZE];
    sprintf(fileName,"%s/chatUsers.txt",pathToFiles);
    users = fopen(fileName, "r");
    if (users == NULL) {
        perror("DONE\n");
    }

    char foundLogin[MAX_BUFFER_SIZE] = "";
    char foundPassword[MAX_BUFFER_SIZE] = "";
    if (countNumOfLines(fileName) > 0) {
        while (!feof(users)) {
            fscanf(users, "%s\t%s", foundLogin, foundPassword);
            if (strcmp(foundLogin, login) == 0) {
                strcpy(password, foundPassword);
                return true;
            }
        }

    }
    fclose(users);
    return false;

}

void log(Client *client, char *login);

void registration(Client *client, char *login) {
    char *msg;
    char password[MAX_BUFFER_SIZE];
    while (checkExistingUser(login, password)) {
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

    sprintf(fileName, "%s/%s", pathToFiles,login);
    mkdir(fileName,0777);

    sprintf(fileName, "%s/%s/%sRequests.txt",pathToFiles, login,login);
    user = fopen(fileName, "w");
    fclose(user);

    sprintf(fileName, "%s/%s/%sFriends.txt",pathToFiles ,login,login);
    user = fopen(fileName, "w");
    fclose(user);

    sprintf(fileName, "%s/%s/%sMessages.txt",pathToFiles, login,login);
    user = fopen(fileName, "w");
    fclose(user);

    sprintf(fileName,"%s/chatUsers.txt",pathToFiles);
    users = fopen(fileName, "a");
    fprintf(users, "%s\t%s\n", login, password);
    fclose(users);
    printf("Vytvoril som usera s loginom %s a heslom %s zapisal ho do suboru\n", login, password);
}

void acceptFriendRequest(Client *client, char *recv, char *send) {

    char option[MAX_BUFFER_SIZE];
    char fileName[MAX_BUFFER_SIZE];
    char infoReq[MAX_BUFFER_SIZE];

    printf("%s posiela ziadost %s o priatelstvo\n", send, recv);
    sprintf(infoReq, "%s vam posiela ziadost o priatelstvo, chcete ho prijat? [A|N]", send);
    write(client->sockfd, infoReq, strlen(infoReq) + 1);
    checkCorrectInput(client,"A","N",option);

    if (strcmp(option, "A") == 0) {
        printf("%s si zvolil moznost A - pridat priatela %s\n", recv, send);
        FILE *friendsFile;
        sprintf(fileName, "%s/%s/%sFriends.txt",pathToFiles, recv,recv);
        friendsFile = fopen(fileName, "a");
        fprintf(friendsFile, "%s\n", send);
        fclose(friendsFile);
        printf("Priatel %s bol pridany do suboru %s\n",send, fileName);

        sprintf(fileName, "%s/%s/%sFriends.txt",pathToFiles, send,send);
        friendsFile = fopen(fileName, "a");
        fprintf(friendsFile, "%s\n", recv);
        fclose(friendsFile);
        printf("Priatel %s bol pridany do suboru %s\n", recv, fileName);
    }

}

void chceckFriendsRequests(Client *client) {
    printf("Idem otvorit friend requesty uzivatela %s\n", client->name);

    char fileName[MAX_BUFFER_SIZE];
    sprintf(fileName, "%s/%s/%sRequests.txt", pathToFiles,client->name,client->name);
    int numOfRequests = countNumOfLines(fileName);

    //TODO check friend request metoda
    printf("Pocet requestov uzivatela %s je %d\n", client->name, numOfRequests);
    if (numOfRequests > 0) {
        char sender[MAX_BUFFER_SIZE];
        FILE *requestFile;
        requestFile = fopen(fileName, "r");

        while (numOfRequests > 0) {
            fscanf(requestFile, "%s\n", sender);
            acceptFriendRequest(client, client->name, sender);
            numOfRequests--;
        }

        fclose(requestFile);
        sprintf(fileName, "%s/%s/%sRequests.txt", pathToFiles,client->name,client->name);
        remove(fileName);
        printf("Uzivatel %s si pozrel vsetky jeho requesty a tak sa zmazal subor %s\n", client->name, fileName);

        FILE *newRequest = fopen(fileName, "w");
        fclose(newRequest);
        printf("Vytvoril som novy subor na requesty uzivatela %s nazvom %s\n",client->name, fileName);
    }
}

bool checkAlreadyLoggedIn(char * login)
{
    for (int i = 0; i < onlineUsers.size; ++i) {
        if(getClient(&onlineUsers,login))
        {
            return true;
        }
    }
    return false;
}

void log(Client *client, char *login) {
    char foundPassword[MAX_BUFFER_SIZE];
    char password[MAX_BUFFER_SIZE];
    char *msg;
    while (!checkExistingUser(login, foundPassword) || checkAlreadyLoggedIn(login)) {
        if(!checkExistingUser(login, foundPassword))
        {
            printf("Uzivatel %s zadal neregistrovany login\n", client->name);
            msg = "Zadany login nie je zaregistrovany, zadaj registrovany login, alebo klikni 1 pre registruaciu!";
        }
        else{
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

void onlineUserMenu(Client *client);

void regLogin(Client *client) {
    char buffer[MAX_BUFFER_SIZE];
    char *msg = "Zadaj co chces robit:\n1.Registracia\n2.Prihlasenie\nPre ukoncenie napis exit";
    write(client->sockfd, msg, strlen(msg) + 1);
    checkCorrectInput(client,"1","2",buffer);


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

    client->name = (char *) malloc(sizeof(login));
    strcpy(client->name, login);
    client->isOnline = true;
    add(&onlineUsers, client);
    if (strcmp(buffer, "2") == 0) {
        chceckFriendsRequests(client);
        FILE *file;
        char fileName[MAX_BUFFER_SIZE];
        sprintf(fileName, "%s/%s/%sRemoveFriends.txt", pathToFiles,client->name,client->name);
        if (file = fopen(fileName, "r")) {
            char removed[MAX_BUFFER_SIZE];
            strcpy(buffer, "Z priatelov si ta odstranili:\n");
            while (!feof(file)) {
                fscanf(file, "%s", removed);
                printf( "Uzivatel %s si odstranil uzivatela %s z priatelov :(\n",removed,  client->name);
                strcat(buffer, removed);
                strcat(buffer, "\n");
            }
            fclose(file);
            remove(fileName);
            strcat(buffer, "Pre navrat do menu stlac lubovolnu klavesu");
            write(client->sockfd, buffer, strlen(buffer) + 1);
            read(client->sockfd, buffer, sizeof(buffer));
        }


    }
    printf("Klient s loginom %s a socketom %d je pripojeny:)\n", client->name, client->sockfd);
    onlineUserMenu(client);


}

bool checkAlreadyInFile(char * fileName, char * login){
    FILE *file;
    printf("Idem sa pokusit otvorit %s\n",fileName);
    file = fopen(fileName, "r");

    char foundLogin[MAX_BUFFER_SIZE] = "";
    bool found = false;
    int numOfLines = countNumOfLines(fileName);
    if (numOfLines> 0)
    {
        for (int i = 0; i < numOfLines; ++i)
        {
            fscanf(file, "%s", foundLogin);
            printf("Precital som %s\n",foundLogin);
            if (strcmp(foundLogin, login) == 0) {
                printf("Nasiel som %s\n",foundLogin);
                found = true;
                break;
            }

        }
    }
    fclose(file);
    printf("checkhol som subor %s\n", fileName);
    printf("V %s result: %d\n",fileName,found);

    return found;
}

bool checkAlreadyFriendsAndRequests(Client *client, char *friendName) {

    //printf("Idem skontrolovat ci je uzivatel %s kamarat s uzivatelom %s resp ci uz si poslali ziadost o priatelstvo\n", client->name, friendName);
    char choose[MAX_BUFFER_SIZE];
    bool exists=false;
    for (int i = 0; i <  client->numOfAddRequests; ++i)
    {
        strcpy(choose, client->addRequests + (i * MAX_BUFFER_SIZE));

        if(strcmp(choose, friendName) == 0)
        {
            exists= true;
            break;
        }

    }

    char friendsFile[MAX_BUFFER_SIZE];
    char requestsFile[MAX_BUFFER_SIZE];
    sprintf(friendsFile, "%s/%s/%sFriends.txt", pathToFiles,friendName,friendName);
    sprintf(requestsFile, "%s/%s/%sRequests.txt", pathToFiles,friendName,friendName);

    printf("Idem skontrolovat ci uzivatel %s uz neziadal %s\n",friendName,client->name);
    printf("Skontrolovane online requesty uzivatela %s, najdeny request od uzivatela %s: %d\n",client->name, friendName, exists);
    printf("Idem skontrolovat subor %s ci sa v nom nachadza uzivatel %s\n",requestsFile, client->name);
    printf("A teda ci uz si poslali ziadost od priatelstvo %d", checkAlreadyInFile(requestsFile,client->name));

    printf("Idem skontrolovat subor %s ci sa v nom nachadza uzivatel %s\n",friendsFile, client->name);
    printf("A teda ci uz su kamarati: %d", checkAlreadyInFile(friendsFile,client->name));

    return exists || checkAlreadyInFile(friendsFile,client->name) || checkAlreadyInFile(requestsFile,client->name);
}


void checkNewAddRequests(Client *client)
{
    char choose[MAX_BUFFER_SIZE];
    printf("Pocet cakajucich ziadosti o priatelstvo uzivatela %s je %d\n", client->name,client->numOfAddRequests);
    for (int i = 0; i <  client->numOfAddRequests; ++i)
    {
        strcpy(choose, client->addRequests + (i * MAX_BUFFER_SIZE));

        if (checkAlreadyFriendsAndRequests(client, choose))
        {
            acceptFriendRequest(client, client->name, choose);
        }
    }
    printf("Aktualny pocet ziadosti o priatelstvo uzivatela %s je %d\n",client->name,client->numOfAddRequests);
    client->numOfAddRequests = 0;
    bzero(client->addRequests, MAX_BUFFER_SIZE * MAX_INBOX_SIZE);

}

void checkNewRemoval(Client *client)
{
    char user[MAX_BUFFER_SIZE];
    char msg[MAX_BUFFER_SIZE];
    strcpy(msg,"Nasledovny uzivatelia si vas odstranili z priatelov:");
    printf("Uzivatel %s si ide skontrolovat ci si ho niekto odstranil z priatelov\n", client->name);
    printf("Uzivatela %s si odstranilo %d uzivatelov su to:\n", client->name,client->numOfRemoveNotifications);
    //TODO ten isty cyklus ako na newAddRequests
    for (int i = 0; i < client->numOfRemoveNotifications; ++i)
    {
        strcpy(user, client->removeNotifications + (i * MAX_BUFFER_SIZE));
        printf("%s\n", user);
        strcat(msg,"\n");
        strcat(msg,user);
    }
    strcat(msg,"\nPre navrat do menu stlac lubovolnu klavesu");
    write(client->sockfd,msg,strlen(msg)+1);
    read(client->sockfd,user,strlen(user)+1);
    client->numOfRemoveNotifications = 0;
    bzero(client->removeNotifications, MAX_BUFFER_SIZE * MAX_INBOX_SIZE);

}

void logout(Client *client) {

    printf("Uzivatel %s ma %d ziadosti o priatelstvo\n",client->name, client->numOfAddRequests);
    if (client->numOfAddRequests > 0) {
        checkNewAddRequests(client);
    }
    printf("Uzivatela %s si %d ludi odstranilo z priatelov\n",client->name, client->numOfRemoveNotifications);
    if  (client->numOfRemoveNotifications>0)
    {
        checkNewRemoval(client);
    }
    client->isOnline = false;

    printf("Uzivatel s loginom %s a socketom %d sa odhlasil\n", client->name, client->sockfd);

    free(client->name);
    regLogin(client);

}

void removeLineFromFile(char * remLogin, char * fileName, bool removeFriend)
{
    FILE *users;
    FILE *newUsers;
    printf("Idem Vymazat riadok zo suboru %s\n", fileName);
    users = fopen(fileName, "r");
    char tmpFileName[MAX_BUFFER_SIZE];
    sprintf(tmpFileName,"%s/tmp.txt",pathToFiles);
    newUsers = fopen(tmpFileName, "w");
    char foundLogin[MAX_BUFFER_SIZE];
    char foundPassword[MAX_BUFFER_SIZE];
    int numOfRegistered= countNumOfLines(fileName);
    for (int i =0; i < numOfRegistered; i++) {
        if(removeFriend)
        {
            fscanf(users, "%s\n", foundLogin);
        }
        else
        {
            fscanf(users, "%s\t%s\n", foundLogin,foundPassword);
        }
        if (strcmp(remLogin, foundLogin) != 0) {
            if(removeFriend)
            {
                fprintf(newUsers, "%s\n", foundLogin);
                printf("Odstranil som %s\n",foundLogin);
            }
            else
                fprintf(newUsers, "%s\t%s\n", foundLogin,foundPassword);
        }

    }
    fclose(users);
    fclose(newUsers);

    remove(fileName);
    rename(tmpFileName, fileName);
}

void removeUserFromFriends(char *userName, char *removingUserName) {

    char fileName[MAX_BUFFER_SIZE];
    sprintf(fileName,"%s/%s/%sFriends.txt",pathToFiles, userName,userName);
    printf("Idem zmazat uzivatela %s zo suboru %s",removingUserName,fileName);
    removeLineFromFile(removingUserName,fileName,true);
    printf("Zmazal som uzivatela %s zo suboru %s",removingUserName,fileName);

}

void removeAccount(Client *client) {

    printf("Idem odstranit uzivatela %s\n",client->name);
    char fileName[MAX_BUFFER_SIZE];
    sprintf(fileName, "%s/%s/%sFriends.txt", pathToFiles,client->name,client->name);
    removeLineFromFile(client->name,fileName,false);
    sprintf(fileName, "%s/%s/%sFriends.txt", pathToFiles,client->name,client->name);
    FILE * file = fopen(fileName,"r");
    int numOfFriends= countNumOfLines(fileName);
    char friend[MAX_BUFFER_SIZE];
    for (int i = 0; i < numOfFriends; ++i) {
        fscanf(file,"%s",friend);
        removeUserFromFriends(friend,client->name);
    }
    remove(fileName);

    sprintf(fileName, "%s/%s/%sRequests.txt", pathToFiles,client->name,client->name);
    remove(fileName);
    sprintf(fileName, "%s/%s/%sMessages.txt",pathToFiles, client->name,client->name);
    remove(fileName);
    sprintf(fileName, "%s/%s", pathToFiles,client->name);
    rmdir(fileName);
    printf("Odstranil som uzivatela %s a vsetky jeho subory spolu s priecinkom\n", client->name);
    client->isOnline = false;
    free(client->name);
    regLogin(client);

}

void printUsers(Client *client, char * fileName, char * contact, int funOption);

void addFriend(Client *client) {
    printf("Uzivatel %s si ide pridat noveho priatela\n", client->name);
    char choose[MAX_BUFFER_SIZE];
    char msg[MAX_BUFFER_SIZE];
    char fileName[MAX_BUFFER_SIZE];
    sprintf(fileName,"%s/chatUsers.txt",pathToFiles);

    int numOfLines = countNumOfLines(fileName);
    if(numOfLines==1)
    {
        printf("Uzivatel %s nema na vyber ziadneho noveho priatela\n", client->name);
        sprintf(msg, "Ziadny uzivatelia na pridanie, pre navrat do menu stlac lubovolnu klavesu");
        write(client->sockfd,msg, strlen(msg)+1);
        read(client->sockfd,choose,sizeof(choose));
        onlineUserMenu(client);
        return;
    }

    printUsers(client,fileName,choose,3);
    Client *choosenUser = getClient(&onlineUsers, choose);

    if (choosenUser) {

        if (choosenUser->isOnline) {
            printf("Pocet novych ziadosti o priatelstvo uzivatela %s je  %d\n", client->name, client->numOfAddRequests);
            printf("Idem skontrolovat ci uzivatel %s uz ziadal uzivatela %s o priatelstvo\n",client->name,choosenUser->name);
            if (choosenUser->numOfAddRequests < MAX_INBOX_SIZE || !checkAlreadyFriendsAndRequests(choosenUser, client->name))
            {
                printf("Uzivatel %s este neziadal uzivatela %s o priatelstvo",client->name,choosenUser->name);
                strcpy(choosenUser->addRequests + (choosenUser->numOfAddRequests * MAX_BUFFER_SIZE), client->name);
                printf("Uzivatel %s si pridal do ziadosti o priatelstvo uzivatela %s\n",choosenUser->name,choosenUser->addRequests);
                (choosenUser->numOfAddRequests)++;

            } else {
                printf("Uzivatelovi %s nie je mozne poslat ziadost\n", choose);
                strcpy(msg,"Uzivatelovi nie je mozne poslat ziadost, vyber si ako chces pokracovat\n1.Spat do menu\n2.Poslat ziadost inemu uzivatelovi");
                write(client->sockfd, msg, strlen(msg) + 1);
                read(choosenUser->sockfd, choose, sizeof(choose));
                if (strcmp(choose, "1") == 0) {
                    onlineUserMenu(client);
                } else {
                    addFriend(client);
                }

            }
        }

    } else {
        char sender[MAX_BUFFER_SIZE];
        strcpy(sender, client->name);
        FILE *choosenFile;
        sprintf(fileName, "%s/%s/%sRequests.txt", pathToFiles,choose,choose);
        choosenFile = fopen(fileName, "a");
        fprintf(choosenFile, "%s\n", sender);
        fclose(choosenFile);
    }
    printf("Uzivatel %s poslal uzivatelovi %s ziadost o priatelstvo\n", client->name, choose);
    printf("Uzivatel %s ma aktualne %d ziadosti o priatelstvo\n",choosenUser->name,choosenUser->numOfAddRequests);

    onlineUserMenu(client);

}

void printUsers(Client *client, char * fileName, char * contact, int funOption)
{
    FILE * file;
    char message[MAX_BUFFER_SIZE];
    char buffer[MAX_BUFFER_SIZE];
    int numOfFriends= countNumOfLines(fileName);

    sprintf(message,"Vyber si uzivatela, alebo stlac 1 pre navrat do menu");
    file= fopen(fileName,"r");
    char users[MAX_BUFFER_SIZE*numOfFriends];
    char user[MAX_BUFFER_SIZE];
    bool print;
    for (int i = 0; i < numOfFriends; ++i) {
        print=false;
        if(funOption == 3)
        {
            fscanf(file, "%s %s", user, buffer);
            printf("Porovnavam %s a %s\n",user,client->name);
            if (strcmp(user, client->name) != 0 && !checkAlreadyFriendsAndRequests(client, user))
            {
                printf("Uzivatel %s nema este uzivatela %s v priateloch\n",user,client->name);
                print=true;
            }
        }
        else
        {
            fscanf(file,"%s\n",user);
            print=true;
        }
        if(print)
        {
            strcat(message,"\n");
            strcat(message,user);

            printf("Precital som %s\n",user);
            strcpy(users + i*MAX_BUFFER_SIZE, user);
            printf("Target uzivatela %s je %s\n", client->name, users + i*MAX_BUFFER_SIZE);

        }

    }
    fclose(file);
    write(client->sockfd,message,strlen(message)+1);
    strcat(message,"\nZadaneho uzivatela nemas v priateloch, vyber si niekoho z vyssie zadanych alebo klikni 1 pre navrat do menu.");
    bool done= false;
    while(!done)
    {
        read(client->sockfd,buffer,sizeof(buffer));
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
    sprintf(fileName, "%s/%s/%sFriends.txt",pathToFiles, client->name,client->name);
    char friend[MAX_BUFFER_SIZE];

    int numOfLines = countNumOfLines(fileName);
    if(numOfLines==0)
    {
        printf("Uzivatel %s nema ziadnych kamaratov, nema si koho odstranit\n", client->name);
        sprintf(friend, "Ziadny priatelia na odstranenie, pre navrat do menu stlac lubovolnu klavesu");
        write(client->sockfd,friend, strlen(friend)+1);
        read(client->sockfd,fileName,sizeof(fileName));
        onlineUserMenu(client);
        return;
    }

    printUsers(client, fileName,friend, 2);
    printf("Uzivatel %s si vybral ze chce odstranit: %s\n",client->name, friend);
    Client *removingUser = getClient(&onlineUsers, friend);
    FILE *friendsFile;
    if (removingUser) {
        strcpy(removingUser->removeNotifications + removingUser->numOfRemoveNotifications * MAX_BUFFER_SIZE, client->name);
        printf("Uzivatel %s je online a tak sa mu notifikacia o tom ze si ho niekto odstranil z priatelov zapisala do pola\n", removingUser->name);
        removingUser->numOfRemoveNotifications++;
    } else {
        sprintf(fileName, "%s/%s/%sRemoveFriends.txt",pathToFiles, friend,friend);
        friendsFile = fopen(fileName, "a");
        fprintf(friendsFile, "%s", client->name);
        fclose(friendsFile);
        printf("Uzivatel %s je offline a tak sa mu uzivatel, ktory si ho odstranil zo zoznamu priatelov zapisal do suboru %s\n", friend, fileName);
    }

    removeUserFromFriends(client->name, friend);
    removeUserFromFriends(friend, client->name);
    printf("Uzivatel %s si odstranil uzivatela %s z priatelov\n", client->name, friend);
    onlineUserMenu(client);
}

void writeMsgAndCopyItToFile(Client * client,char * encryptedMsg){

    char msg[MAX_BUFFER_SIZE];
    char buffer[MAX_BUFFER_SIZE];
    sprintf(msg,"Zadaj spravu, ktoru chces poslat:");
    write(client->sockfd,msg, strlen(msg)+1);
    read(client->sockfd,msg,MAX_BUFFER_SIZE);
    printf("Uzivatel %s posiela spravu %s uzivatelovi do suboru\n",client->name,msg);

    sprintf(buffer,"%s: %s",client->name,msg);

    for(int i = 0; (i < MAX_BUFFER_SIZE && buffer[i] != '\0'); i++)
    {
        buffer[i] = buffer[i] + 3;
    }
    strcpy(encryptedMsg,buffer);
}

void sendMsg(Client *client) {

    printf("Uzivatel %s ide poslat spravu niekomu zo zoznamu priatelov\n", client->name);
    FILE * file;
    char receiver[MAX_BUFFER_SIZE];
    char buffer[MAX_BUFFER_SIZE];
    char msg[MAX_BUFFER_SIZE];
    sprintf(buffer,"%s/%s/%sFriends.txt",pathToFiles,client->name,client->name);

    int numOfLines = countNumOfLines(buffer);
    if(numOfLines==0)
    {
        printf("Uzivatel %s nema komu napisat, najprv si musi niekoho pridat\n", client->name);
        sprintf(msg, "Nemas komu napisat, najprv si niekoho pridaj:)\nVyber si co chces robit\n1.Pridat priatela\n2.Navrat do menu");
        write(client->sockfd,msg, strlen(msg)+1);
        read(client->sockfd,buffer,sizeof(buffer));
        if(strcmp(buffer,"1")==0)
        {
            addFriend(client);
        }
        else
        {
            onlineUserMenu(client);
        }
        return;
    }

    printUsers(client, buffer, receiver, 1);

    printf("Uzivatel %s si zvolil ze chce spravu poslat uzivatelovi %s\n",client->name, receiver);
    sprintf(buffer,"%s/%s/%sMessages.txt",pathToFiles,receiver,receiver);
    file = fopen(buffer,"a");

    writeMsgAndCopyItToFile(client,buffer);
    fprintf(file,"%s\n",buffer);
    fclose(file);
    onlineUserMenu(client);
}

void checkMsgInbox(Client * client)
{
    printf("Uzivatel %s si ide pozriet neprecitane spravy\n", client->name);
    FILE * file;
    char buffer[MAX_BUFFER_SIZE];
    sprintf(buffer,"%s/%s/%sMessages.txt",pathToFiles,client->name,client->name);
    int numOfMsgs= countNumOfLines(buffer);
    if(numOfMsgs==0)
    {
        printf("Uzivatel %s nema ziadne neprecitane spravy\n", client->name);
        sprintf(buffer,"Nemas ziadne neprecitane spravy, pre navrat do menu stlac lubovolnu klavesu");
        write(client->sockfd,buffer, strlen(buffer)+1);
        read(client->sockfd,buffer,sizeof(buffer));
        onlineUserMenu(client);
        return;
    }

    file= fopen(buffer,"r");
    char msg[MAX_BUFFER_SIZE*numOfMsgs];
    bzero(msg,MAX_BUFFER_SIZE);
    for (int i = 0; i < numOfMsgs; ++i) {
        fscanf(file,"%s",buffer);
        for(int j = 0; (j < MAX_BUFFER_SIZE && buffer[j] != '\0');j++)
        {
            buffer[j] = buffer[j] - 3; //the key for encryption is 3 that is added to ASCII value
        }
        strcat(msg,buffer);
        strcat(msg,"\n");
    }
    printf("Vsetky neprecitane spravy uzivatela %s boli desifrovane a vypisane na obrazovku\n", client->name);
    fclose(file);

    strcat(msg,"Pre navrat do menu stlac lubovolnu klaves");
    write(client->sockfd,msg, strlen(msg)+1);
    read(client->sockfd,buffer,sizeof(buffer));
    sprintf(buffer,"%s/%s/%sMessages.txt",pathToFiles,client->name,client->name);
    file=fopen(buffer,"w");
    fclose(file);
    printf("Subor neprecitanych sprav uzivatela %s bol premazany\n", client->name);
    onlineUserMenu(client);
}


void getFileName(char * fileName, char * result){
    char * savePtr=NULL;
    char * dump;
    for (dump = strtok_r(fileName, "/", &savePtr); dump != NULL; dump = strtok_r(NULL, "/", &savePtr)){
        strcpy(result,dump);
        printf("token:%s\n", dump);
    }
    printf("NEW FILE NAME %s\n",result);

}

void copyFileContent(char * originalFilePath, char * copyFilePath){

    FILE* file=fopen(originalFilePath,"r");
    FILE* newFile=fopen(copyFilePath,"w");
    int numOfLines= countNumOfLines(originalFilePath);
    printf("Subor z ktoreho kopcim je: %s\n",originalFilePath);
    char buffer[MAX_BUFFER_SIZE];
    for(int i=0; i <numOfLines;i++)
    {
        fscanf(file, "%[^\n]\n", buffer);
        printf("Data from the file:%s\n", buffer);
        fprintf(newFile,"%s\n",buffer);
        printf("Zapisal som do suboru %s\n",buffer);
    }
    fclose(file);
    fclose(newFile);
}

void checkFile(Client* client, char * originalFile){
    FILE * file;
    file=fopen(originalFile,"r");
    char msg[MAX_BUFFER_SIZE];
    char buffer[MAX_BUFFER_SIZE];
    while(file==NULL)
    {
        strcpy(msg,"Nepodarilo sa otvorit subor, vyber si ako chces pokracovat\n1.Navrat do menu\n2.Poslat iny subor");
        write(client->sockfd,msg,strlen(msg)+1);
        read(client->sockfd,buffer,sizeof(buffer));
        if(strcmp(buffer,"1")==0)
        {
            onlineUserMenu(client);
            return;
        }
        else
        {
            file=fopen(originalFile,"r");
        }

    }
    fclose(file);
}


void sendFile(Client *client) {

    printf("Uzivatel %s ide poslat niekomu subor\n", client->name);
    char msg[MAX_BUFFER_SIZE]="Zadaj cestu k suboru a nazov suboru, ktory chces poslat:";
    char buffer[MAX_BUFFER_SIZE];
    write(client->sockfd,msg,strlen(msg)+1);
    read(client->sockfd,buffer,sizeof(buffer));
    printf("Uzivatel %s si vybral subor a cesta k nemu je %s\n",client->name, buffer);

    char fileName[MAX_BUFFER_SIZE];
    strcpy(fileName,buffer);

    strcpy(msg,"Zadaj meno uzivatela, ktoremu chces poslat subor:");
    write(client->sockfd,msg,strlen(msg)+1);
    strcat(msg,"\nZadaj uzivatela,ktory existuje!");
    read(client->sockfd,buffer,sizeof(buffer));
    char password[MAX_BUFFER_SIZE];
    while(!checkExistingUser(buffer,password))
    {
        write(client->sockfd,msg,strlen(msg)+1);
        read(client->sockfd,buffer,sizeof(buffer));
    }

    printf("Uzivatel %s si vybral ze chce poslat subor uzivatelovi %s\n", client->name, buffer);
    checkFile(client,fileName);
    char name[MAX_BUFFER_SIZE];
    char originalFileName[MAX_BUFFER_SIZE];
    strcpy(originalFileName,fileName);
    getFileName(fileName,name);
    char pathToNewFile[MAX_BUFFER_SIZE];
    sprintf(pathToNewFile,"%s/%s/%s",pathToFiles,buffer,name);
    copyFileContent(originalFileName,pathToNewFile);
    printf("Uzivatel %s uspesne poslal subor s nazvom %s uzivatelovi %s, ktory sa nachadza v jeho priecinku\n", client->name, name, buffer);
    onlineUserMenu(client);

}

void makeGroup(Client * client) {

    printf("Uzivatel %s ide vytvorit skupinu, ktorej posle spravu alebo subor\n", client->name);
    char * msg;
    char fileName[MAX_BUFFER_SIZE];
    char tmpFileName[MAX_BUFFER_SIZE];
    sprintf(fileName,"%s/%s/%sFriends.txt",pathToFiles,client->name,client->name);
    sprintf(tmpFileName,"%s/%s/%stmp.txt",pathToFiles,client->name,client->name);
    copyFileContent(fileName,tmpFileName);
    int maxMembers=countNumOfLines(fileName);
    char members[maxMembers*MAX_BUFFER_SIZE];
    int numOfMembers=0;
    char buffer[MAX_BUFFER_SIZE];

    do
    {
        msg="Vyber si co chces robit:\nPridat clena do skupiny(P)\nSkoncit zadavanie(S)";
        write(client->sockfd,msg, strlen(msg)+1);
        checkCorrectInput(client,"S","P",buffer);

        if(strcmp(buffer,"P")==0)
        {
            printf("Uzivatel si vybral pridat dalsieho uzivatela do skupiny\n");
            char member[MAX_BUFFER_SIZE];
            printUsers(client,tmpFileName,member,4);
            if(strcmp(member,"1")!=0)
            {
                printf("Do skupiny bol pridany uzivatel %s\n",member);
                strcpy(members + numOfMembers * MAX_BUFFER_SIZE, member);
                removeLineFromFile(member,tmpFileName,true);
                numOfMembers++;
            }
        }

    }while(strcmp(buffer,"S")!=0 && numOfMembers < maxMembers);

    remove(tmpFileName);

    printf("Uzivatel %s si ide vybrat ci chce poslat vsetkym clenom skupiny spravu alebo subor\n", client->name);

    msg="Vyber si co chces poslat:\n1.Spravu\n2.Subor";
    write(client->sockfd,msg, strlen(msg)+1);
    checkCorrectInput(client,"1","2",buffer);

    if(strcmp(buffer,"1")==0 )
    {
        char message[MAX_BUFFER_SIZE];
        writeMsgAndCopyItToFile(client,message);
        for (int i = 0; i < numOfMembers; ++i) {
            sprintf(fileName,"%s/%s/%sMessages.txt",pathToFiles,members+i*MAX_BUFFER_SIZE,members+i*MAX_BUFFER_SIZE);
            FILE * file = fopen(fileName,"a");
            fprintf(file,"%s\n",message);
            fclose(file);
            printf("Sprava bola zasifrovana a zapisana do suboru sprav uzivatela %s\n", members+i*MAX_BUFFER_SIZE);
        }
    }
    else
    {
        printf("Uzivatel %s si zvolil ze chce poslat vsetkym clenom skupiny subor\n", client->name);
        msg="Zadaj cestu k suboru a nazov suboru, ktory chces polat skupine:";
        write(client->sockfd,msg, strlen(msg)+1);
        read(client->sockfd,buffer,sizeof(buffer));

        char orifinalFile[MAX_BUFFER_SIZE];
        strcpy(orifinalFile,buffer);
        checkFile(client,buffer);
        char name[MAX_BUFFER_SIZE];
        getFileName(buffer,name);
        char pathToNewFile[MAX_BUFFER_SIZE];

        for (int i = 0; i < numOfMembers; ++i) {
            printf("Uzivatel %s posiela subor s nazvom %s uzivatelovi %s\n",client->name, name, members+MAX_BUFFER_SIZE*i);
            sprintf(pathToNewFile,"%s/%s/%s",pathToFiles,members+MAX_BUFFER_SIZE*i,name);
            copyFileContent(orifinalFile,pathToNewFile);
        }
    }
    onlineUserMenu(client);

}


void onlineUserMenu(Client *client) {

    printf("Uzivatela %s si odstanilo %d uzivatelov\n",client->name,client->numOfRemoveNotifications);
    if  (client->numOfRemoveNotifications>0)
    {
        checkNewRemoval(client);
    }
    printf("Uzivatel %s ma %d ziadosti o priatelstvo\n",client->name,client->numOfAddRequests);
    if (client->numOfAddRequests > 0) {
        checkNewAddRequests(client);
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
    }else if (strcmp(option, "7") == 0) {
        sendFile(client);
    } else if(strcmp(option, "8") == 0){
        makeGroup(client);
    }else{
        onlineUserMenu(client);
    }
}

void *handleClient(void *pdata) {
    Client *dataC = pdata;
    regLogin(dataC);
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

    init(&onlineUsers);
    initListeningSocket(&port, &listenfd, &serv_addr);

    int id = 0;
    pthread_t client;

    pthread_mutex_init(&clientMutex, NULL);

    int numOfClients = 5;
    while (true) {
        numOfloggedIn++;
        initConnSocket(&listenfd, &connfd, &cli_addr);
        printf("Joinul som sa s clientom\n");
        Client *newClient = (Client *) malloc(sizeof(Client));
        newClient->id = id++;
        newClient->sockfd = connfd;
        newClient->cl_address = cli_addr;
        newClient->numOfAddRequests = 0;
        newClient->numOfRemoveNotifications=0;

        pthread_create(&client, NULL, handleClient, newClient);
        pthread_detach(client);
        numOfClients--;
       // free(newClient);
    }

    pthread_mutex_destroy(&clientMutex);
    close(connfd);
    printf("Zavrel som connfd\n");
    close(listenfd);
    printf("Zavrel som listen\n");
    return 0;


}

