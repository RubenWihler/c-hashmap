#include "hashmap.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

typedef struct _node_t {
    void* key;
    void* value;
    struct _node_t* next;
} node_t;

struct _hashmap_t {
    size_t capacity;
    size_t key_size;
    size_t value_size;
    size_t count;
    node_t** table;

    hash_fn_t hash_fn;
    destroy_fn_t key_destroy_fn;
    destroy_fn_t value_destroy_fn;
};

static const size_t minimal_capacity = 1;

static node_t* node_create(const void *key, const void *value, const size_t key_size, const size_t value_size);
static void node_destroy(node_t *node);

hashmap_t* hashmap_create(size_t initial_capacity, const size_t key_size, const size_t value_size,
                          hash_fn_t hash_fn, destroy_fn_t key_destroy_fn, destroy_fn_t value_destroy_fn)
{
    assert(key_size > 0 && value_size > 0 && hash_fn != NULL);
    if(initial_capacity < minimal_capacity) initial_capacity = minimal_capacity;

    //allocation pour la hashmap
    hashmap_t *hashmap = malloc(sizeof(*hashmap));
    if(!hashmap) return (perror("malloc"), NULL);
    memset(hashmap, 0, sizeof(*hashmap));

    hashmap->capacity = initial_capacity;
    hashmap->key_size = key_size;
    hashmap->value_size = value_size;
    hashmap->count = 0;
    hashmap->hash_fn = hash_fn;
    hashmap->key_destroy_fn = key_destroy_fn;
    hashmap->value_destroy_fn = value_destroy_fn;
    
    //allocatino pour le tableau qui va contenir les donnees
    hashmap->table = calloc(hashmap->capacity, sizeof(*hashmap->table));
    if(!hashmap->table) return (perror("calloc"), free(hashmap), NULL);

    return hashmap;
}

void hashmap_destroy(hashmap_t *hm)
{
    for(size_t i = 0; i < hm->capacity; i++)
    {
        node_t *current = hm->table[i];
        while(current != NULL)
        {
            node_t *tmp = current;
            current = current->next;
            
            if(hm->key_destroy_fn != NULL)
                hm->key_destroy_fn(tmp->key);

            if(hm->value_destroy_fn != NULL)
                hm->value_destroy_fn(tmp->value);

            node_destroy(tmp);
        }
    }

    free(hm->table);
    free(hm);
}

void* hashmap_get(hashmap_t *hm, const void* key)
{
    size_t index = hm->hash_fn(key, hm->key_size) % hm->capacity;
    node_t *current = hm->table[index];

    while(current != NULL)
    {
        if(memcmp(key, current->key, hm->key_size) == 0)
            return current;

        current = current->next;
    }

    return NULL;
}

void hashmap_add(hashmap_t *hm, const void* key, const void* value)
{
    size_t index = hm->hash_fn(value, hm->key_size) % hm->capacity;
    node_t *node = node_create(key, value, hm->key_size, hm->value_size);
    if(node == NULL) return;

    hm->count++;

    //todo: auto_growth

    if(hm->table[index] != NULL)
    {
        node_t *current = hm->table[index];

        while(current->next != NULL)
            current = current->next;

        current->next = node;
        return;
    }

    hm->table[index] = node;
}

bool hashmap_remove(hashmap_t *hm, const void *key)
{
    size_t index = hm->hash_fn(key, hm->key_size) % hm->capacity;
    node_t *current = hm->table[index];
    node_t *prev = NULL;

    while(current != NULL)
    {
        if(memcmp(key, current->key, hm->key_size) == 0)
        {
            if(prev != NULL) 
            {
                prev->next = current->next;
            }    
            else
            {
                hm->table[index] = current->next;
            }
            
            //destroy key
            if(hm->key_destroy_fn != NULL) 
                hm->key_destroy_fn(current->key);
            
            //destroy value
            if(hm->value_destroy_fn != NULL)
                hm->value_destroy_fn(current->value);

            node_destroy(current);
            hm->count--;
            return true;
        }

        prev = current;
        current = current->next;
    }

    return false;
}

void hashmap_print(hashmap_t *hm, print_fn_t print_key_fn, print_fn_t print_value_fn)
{
    printf("(hashmap):\n");
    printf("{\n");
    printf("    key_size: %zu bytes\n", hm->key_size);
    printf("    value_size: %zu bytes\n", hm->value_size);
    printf("    capacity: %zu\n", hm->capacity);
    printf("    count: %zu\n", hm->count);
    printf("    table:\n");
    printf("    [\n");

    for(size_t i = 0; i < hm->capacity; i++)
    {
        node_t *current = hm->table[i];
        while(current != NULL)
        {
            printf("\t");
            printf("(%zu) : ", i);
            print_key_fn(current->key);
            printf("  =>  ");
            print_value_fn(current->value);

            if(i < hm->capacity - 1 || current->next != NULL)
            {
                printf(",\n");
            }

            current = current->next;
        }
    }

    printf("\n    ]\n");
    printf("}\n");
}

static void resize(hashmap_t *hm, size_t capacity)
{
    if(capacity < minimal_capacity) capacity = minimal_capacity;
    assert(false && "not implemented yet!");
}


static node_t* node_create(const void *key, const void *value, const size_t key_size, const size_t value_size)
{
    node_t *node = malloc(sizeof(*node));
    if(!node) return (perror("malloc"), NULL);

    node->key = malloc(key_size);
    if(!node->key) return (perror("malloc"), free(node), NULL);

    node->value = malloc(value_size);
    if(!node->value) return (perror("malloc"), free(node->key), free(node), NULL);

    memcpy(node->key, key, key_size);
    memcpy(node->value, value, value_size);
    node->next = NULL;

    return node;
}

static void node_destroy(node_t *node)
{
    free(node->key);
    free(node->value);
    free(node);
}
