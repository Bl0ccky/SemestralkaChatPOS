#include "arrayList.h"

#define INIT_CAPACITY 5



static _Bool ensureCapacity(ArrayList*array)
{
    if(array->size >= array->capacity)
    {
        Client* pom = realloc(array->array, 2 * array->capacity * sizeof(Client));
        if(pom != NULL)
        {
            array->array = &pom;
            array->capacity *= 2;
            return true;
        }
        else
            return false;
    }
    else
        return true;    //kapacita je dostatocna!
}

void init(ArrayList*array)
{
    array->capacity = INIT_CAPACITY;
    array->size = 0;
    array->array = calloc(array->capacity, sizeof(Client));
}
void dispose(ArrayList* array)
{
    free(array->array);
    array->array = NULL;
    array->size = 0;
    array->capacity = 0;
}

void printNames(const ArrayList* array)
{
    for(int i = 0; i < array->size; i++)
    {
        printf("%s ", array->array[i]->name);
    }
    printf("\n");
}

void add(ArrayList* array, Client * data)
{
    if(ensureCapacity(array))
    {
        array->array[array->size++] = data;
    }
}

Client * getClient(const ArrayList *array, char * name) {
    for(int i = 0; i < array->size; i++)
    {
        if(strcmp(name, array->array[i]->name)==0)
        {
            return array->array[i];
        }
    }
    return NULL;
}



_Bool tryInsert(ArrayList* array, Client * data, int pos)
{
    if(pos < 0 || pos > array->size)
    {
        return false;
    }

    if(ensureCapacity(array))
    {
        memmove(array->array + pos + 1, array->array + pos, (array->size - pos) * sizeof(Client));
        array->array[pos] = data;
        array->size++;
        return true;
    }

    else
        return false;

}

void removeClient(ArrayList *array, char *name)
{
    for (int i = 0; i < array->size; ++i)
    {
        if (strcmp(name, array->array[i]->name) == 0)
        {
            array->array[i] = NULL;
        }
    }
}





