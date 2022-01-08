#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "server.h"


#ifndef CHAT_ARRAYLIST_H
#define CHAT_ARRAYLIST_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ArrayList
{
    Client ** array;
    int capacity;
    int size;
}ArrayList;



void init(ArrayList*array);
void dispose(ArrayList*array);
Client * getClient(const ArrayList*array, char * name);
void removeClient(ArrayList*array, char * name);
void printNames(const ArrayList*array);
void add(ArrayList*array, Client * data);


_Bool tryInsert(ArrayList*array, Client * data, int pos);
_Bool trySet(ArrayList*array, int pos, char * data);
_Bool tryGet(ArrayList*array, int pos, char ** data);
_Bool tryRemove(ArrayList*array, int pos, char ** data);
_Bool tryCopy(const ArrayList*src, ArrayList*dest); //využite funkciu memcpy
void readFromTxt(ArrayList*array, FILE *txtFile);
void writeToTxt(const ArrayList*array, FILE *txtFile);


#ifdef __cplusplus
}
#endif

#endif //CHAT_ARRAYLIST_H
