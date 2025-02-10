#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashmap/hashmap.h"

typedef struct {
    struct {
        char *name;
        int age;
    } person;
    
    unsigned char rating;
    double money;
} user_t;

user_t create_user(const char *name, const int age, const unsigned char rating, const double money);
void destroy_user(void *data);
void print_user(const void *data);
void* alloc_copy_user(const void *element, const size_t size);


int main(void)
{
    //create some users
    user_t ruben = create_user("Ruben", 19, 5, 999999999999999);
    user_t thais = create_user("Thaïs", 17, 5, 9991232.9);
    user_t iseut = create_user("Iseut", 15, 5, 9999423.2);
    user_t jules = create_user("Jules", 31, 2, 432.234);
    user_t jeane = create_user("Jeane", 46, 1, 723.2);

    hashmap_t *hm = hashmap_create(HASHMAP_DEFAULT_CAPACITY, HASH_FUNC_DEFAULT, sizeof(char*), sizeof(user_t));
    hashmap_set_fn_compare(hm, HASHMAP_COMPARE_STRING);
    hashmap_set_fn_alloc_copy_key(hm, HASHMAP_ALLOC_COPY_STRING);
    hashmap_set_fn_alloc_copy_value(hm, alloc_copy_user);
    hashmap_set_fn_destroy_value(hm, destroy_user);

    hashmap_add(hm, ruben.person.name, &ruben);
    hashmap_add(hm, thais.person.name, &thais);
    hashmap_add(hm, iseut.person.name, &iseut);
    hashmap_add(hm, jules.person.name, &jules);
    hashmap_add(hm, jeane.person.name, &jeane);
    hashmap_print(hm, HASHMAP_PRINT_STRING, print_user);
    
    //we can free users here, cause the hashmap has copied them
    //(in this case, we allocated them on the stack, only the name is on the heap)
    free(ruben.person.name);
    free(thais.person.name);
    free(iseut.person.name);
    free(jules.person.name);
    free(jeane.person.name);

    hashmap_remove(hm, "Jeane");
    hashmap_remove(hm, "Jules");
    hashmap_print(hm, HASHMAP_PRINT_STRING, print_user);

    if(hashmap_get(hm, "Ruben") == NULL) printf("Ruben not found\n");
    if(hashmap_get(hm, "Thaïs") == NULL) printf("Thaïs not found\n");
    if(hashmap_get(hm, "Iseut") == NULL) printf("Iseut not found\n");
    if(hashmap_get(hm, "Jules") == NULL) printf("Jules not found\n");
    if(hashmap_get(hm, "Jeane") == NULL) printf("Jeane not found\n");

    hashmap_destroy(hm);

    return 0;
}

user_t create_user(const char *name, const int age, const unsigned char rating, const double money)
{
    user_t user = {
        .person = {
            .name = strdup(name),
            .age = age
        },
        .rating = rating,
        .money = money
    };

    if(!user.person.name) perror("strdup");

    return user;
}


void destroy_user(void *data)
{
    user_t *user = (user_t*)data;
    free(user->person.name);
    free(user);
}

void print_user(const void *data)
{
    user_t *user = (user_t*)data;
    printf("{ name: ");
    HASHMAP_PRINT_STRING(user->person.name);
    printf(", age: %d, ", user->person.age);
    printf("rating: %u/5, ", user->rating);
    printf("money: %.2f$ }", user->money);
}

void* alloc_copy_user(const void *element, const size_t size)
{
    user_t *user = (user_t*)element;

    user_t *cpy = malloc(size);
    if(!cpy) return NULL;//perror is not necessary, because it is already handled by hashmap_add
    
    memcpy(cpy, user, size);
    cpy->person.name = strdup(user->person.name);
    
    return (void*)cpy;
}


