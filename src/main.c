#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include "hashmap/hashmap.h"

size_t hash_str(const void* key, const size_t size)
{
    char* ptr = (char*)key;
    size_t hash = 0;

    for (size_t i = 0; i < size; i++)
    {
        hash = hash * 31 + ptr[i];
    }

    return hash;
}

void print_str(const void *data)
{
    printf("\"%s\"", (char*)data);
}

void print_int(const void *data)
{
    printf("%d", *((int*)data));
}

int main(void)
{
    hashmap_t *hm = hashmap_create(103, 20 * sizeof(char), sizeof(int), hash_str, NULL, NULL);

    hashmap_add(hm, "Ruben", &(int){19});
    hashmap_add(hm, "Thais", &(int){17});
    hashmap_add(hm, "Iseut", &(int){15});

    for(int i = 0; i < 100; i++)
    {
        char str[20];
        sprintf(str, "element: %d", i);

        hashmap_add(hm, str, &i);
    }

    hashmap_print(hm, print_str, print_int);

    hashmap_destroy(hm);

    return 0;
}

