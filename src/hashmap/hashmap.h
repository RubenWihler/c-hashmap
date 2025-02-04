#ifndef __HASHMAP_H__
#define __HASHMAP_H__

#include <stddef.h>
#include <stdbool.h>

typedef struct _hashmap_t hashmap_t;

typedef size_t (*hash_fn_t)(const void* key, const size_t size);
typedef void (*print_fn_t)(const void *element);
typedef void (*destroy_fn_t)(void *element);

hashmap_t* hashmap_create(size_t initial_capacity, const size_t key_size, const size_t value_size,
                          hash_fn_t hash_fn, destroy_fn_t key_destroy_fn, destroy_fn_t value_destroy_fn);

void hashmap_destroy(hashmap_t *hm);

void* hashmap_get(hashmap_t *hm, const void* key);

void hashmap_add(hashmap_t *hm, const void* key, const void* value);

bool hashmap_remove(hashmap_t *hm, const void *key);

void hashmap_print(hashmap_t *hm, print_fn_t print_key_fn, print_fn_t print_value_fn);

#endif
