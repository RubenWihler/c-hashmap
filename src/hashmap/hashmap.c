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
    compare_fn_t compare_fn;
    destroy_fn_t key_destroy_fn;
    destroy_fn_t value_destroy_fn;
    alloc_copy_fn_t key_alloc_copy_fn;
    alloc_copy_fn_t value_alloc_copy_fn;
};

static const size_t minimal_capacity = 2;
static const float min_load_balance_threshold = 0.25f;
static const float max_load_balance_threshold = 0.75f;

static inline size_t get_auto_growth_new_capacity(const hashmap_t *hm)
{ return hm->capacity + (hm->capacity >> 1); } //+50%

static inline size_t get_auto_shrink_new_capacity(const hashmap_t *hm)
{ return hm->capacity >> 1; } //-50%

//resize
static void auto_grow(hashmap_t *hm);
static void auto_shrink(hashmap_t *hm);
static void resize(hashmap_t *hm, size_t capacity);

//node management
static node_t* node_create(const hashmap_t *hm, const void *key, const void *value);
static void node_destroy(const hashmap_t *hm, node_t *node);

//default functions
static void* default_alloc_copy_fn(const void *element, const size_t size);
static const compare_fn_t default_compare_fn = memcmp;
static const destroy_fn_t default_destroy_fn = free;

hashmap_t* hashmap_create(size_t initial_capacity, hash_fn_t hash_fn, 
                          const size_t key_size, const size_t value_size)
{
    assert(key_size > 0 && value_size > 0);

    //setting default values
    if(initial_capacity < minimal_capacity) initial_capacity = minimal_capacity;
    if(hash_fn == NULL) hash_fn = HASH_FUNC_DEFAULT;

    //allocation pour la hashmap
    hashmap_t *hashmap = malloc(sizeof(*hashmap));
    if(!hashmap) return (perror("malloc"), NULL);

    //initialisation des valeurs
    hashmap->capacity = initial_capacity;
    hashmap->key_size = key_size;
    hashmap->value_size = value_size;
    hashmap->count = 0;
    hashmap->hash_fn = hash_fn;

    //initialisation des fonctions par defaut
    hashmap->compare_fn = default_compare_fn;
    hashmap->key_destroy_fn = default_destroy_fn;
    hashmap->value_destroy_fn = default_destroy_fn;
    hashmap->key_alloc_copy_fn = default_alloc_copy_fn;
    hashmap->value_alloc_copy_fn = default_alloc_copy_fn;

    //allocation pour le tableau qui va contenir les donnees
    hashmap->table = calloc(hashmap->capacity, sizeof(*hashmap->table));
    if(!hashmap->table) return (perror("calloc"), free(hashmap), NULL);

    return hashmap;
}

void hashmap_destroy(hashmap_t *hm)
{
    //on iterere sur chaque noeud et les detruire
    for(size_t i = 0; i < hm->capacity; i++)
    {
        node_t *current = hm->table[i];
        while(current != NULL)
        {
            node_t *tmp = current;
            current = current->next;
            node_destroy(hm, tmp);
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
        if(hm->compare_fn(key, current->key, hm->key_size) == 0)
            return current->value;

        current = current->next;
    }

    return NULL;
}

void* hashmap_add(hashmap_t *hm, const void* key, const void* value)
{
    //on verifie si la clef existe deja
    void* existing_value = hashmap_get(hm, key);
    if(existing_value != NULL) return existing_value;
    
    //on resize avant d'ajouter l'element
    //cela nous permet de ne pas avoir a rehasher l'element
    hm->count++;
    auto_grow(hm);

    //on ajoute l'element
    size_t index = hm->hash_fn(key, hm->key_size) % hm->capacity;
    node_t *node = node_create(hm, key, value);
    if(node == NULL) return (hm->count--, NULL);//decrement count (mais pas besoin de shrink)

    //on ajoute le noeud en tete de la liste chainée
    //(on rest a O(1) pour l'ajout)
    node->next = hm->table[index];
    hm->table[index] = node;

    return node->value;
}

bool hashmap_remove(hashmap_t *hm, const void *key)
{
    size_t index = hm->hash_fn(key, hm->key_size) % hm->capacity;
    node_t *current = hm->table[index];
    node_t *prev = NULL;

    while(current != NULL)
    {
        if(hm->compare_fn(key, current->key, hm->key_size) == 0)
        {
            if(prev != NULL) //si le noeud n'est pas le premier de la liste
            {
                prev->next = current->next;
            }    
            else //si le noeud est le premier de la liste
            {
                hm->table[index] = current->next;
            }

            node_destroy(hm, current);
            hm->count--;
            auto_shrink(hm);
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
    printf("    load_balance: %.2f\n", (float)hm->count / hm->capacity);
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

size_t hashmap_count(hashmap_t *hm)
{ return hm->count; }

size_t hashmap_capacity(hashmap_t *hm)
{ return hm->capacity; }

void hashmap_set_compare_fn(hashmap_t *hm, compare_fn_t compare_fn)
{ hm->compare_fn = compare_fn; }

void hashmap_set_key_alloc_copy_fn(hashmap_t *hm, alloc_copy_fn_t key_alloc_fn)
{ hm->key_alloc_copy_fn = key_alloc_fn; }

void hashmap_set_value_alloc_copy_fn(hashmap_t *hm, alloc_copy_fn_t value_alloc_fn)
{ hm->value_alloc_copy_fn = value_alloc_fn; }

void hashmap_set_key_destroy_fn(hashmap_t *hm, destroy_fn_t key_destroy_fn)
{ hm->key_destroy_fn = key_destroy_fn; }

void hashmap_set_value_destroy_fn(hashmap_t *hm, destroy_fn_t value_destroy_fn)
{ hm->value_destroy_fn = value_destroy_fn; }


static void auto_grow(hashmap_t *hm)
{
    //si le load balance est trop elevé on resize
    if(((float)hm->count / hm->capacity) > max_load_balance_threshold)
    {
        size_t new_capacity = get_auto_growth_new_capacity(hm);
        resize(hm, new_capacity);
    }
}

static void auto_shrink(hashmap_t *hm)
{
    //si le load balance est trop bas on resize
    if(((float)hm->count / hm->capacity) < min_load_balance_threshold)
    {
        size_t new_capacity = get_auto_shrink_new_capacity(hm);
        resize(hm, new_capacity);
    }
}

static void resize(hashmap_t *hm, size_t new_capacity)
{
    if(new_capacity < minimal_capacity) new_capacity = minimal_capacity;

    //allocation pour le nouveau tableau
    node_t **new_table = calloc(new_capacity, sizeof(*new_table));
    if(!new_table){ perror("calloc"); return; }

    //vu que la capacité change, on doit rehasher les noeuds 
    //(car l'index = hash mod capacité)
    for(size_t i = 0; i < hm->capacity; i++)
    {
        node_t *current = hm->table[i];
        while(current != NULL)
        {
            size_t index = hm->hash_fn(current->key, hm->key_size) % new_capacity;
            node_t *next = current->next;

            current->next = new_table[index];
            new_table[index] = current;

            current = next;
        }
    }

    free(hm->table);
    hm->table = new_table;
    hm->capacity = new_capacity;
}

static void* default_alloc_copy_fn(const void *element, const size_t size)
{
    void *copy = malloc(size);
    if(!copy) return (perror("malloc"), NULL);

    memcpy(copy, element, size);
    return copy;
}

static node_t* node_create(const hashmap_t *hm, const void *key, const void *value)
{
    //allocation pour le noeud
    node_t *node = malloc(sizeof(*node));
    if(!node) return (perror("malloc"), NULL);

    //allocation pour la clef
    node->key = hm->key_alloc_copy_fn(key, hm->key_size);
    if(!node->key) return (free(node), NULL);

    //allocation pour la valeur
    node->value = hm->value_alloc_copy_fn(value, hm->value_size);
    if(!node->value) return (hm->key_destroy_fn(node->key), free(node), NULL);

    node->next = NULL;
    return node;
}

static inline void node_destroy(const hashmap_t *hm, node_t *node)
{
    hm->key_destroy_fn(node->key);
    hm->value_destroy_fn(node->value);
    free(node);
}

//--------------- HASH FUNCTIONS ---------------//
//source for djb2 and sdbm: http://www.cse.yorku.ca/~oz/hash.html

size_t hashmap_hash_func_djb2(const void* key, const size_t size)
{
    char *str = (char*)key;
    size_t hash = 5381;

    while(*str)
        hash = ((hash << 5) + hash) + *str++; /* hash * 33 + c */

    return hash;
}

size_t hashmap_hash_func_sdbm(const void* key, const size_t size)
{
    char *str = (char*)key;
    size_t hash = 0;

    while(*str)
        hash = *str++ + (hash << 6) + (hash << 16) - hash;

    return hash;
}

size_t hashmap_hash_func_id(const void* key, const size_t size)
{
    return *(size_t*)key;
}
