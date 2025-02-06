/*
 *  Author : Wihler Ruben
 *  Date : 2025-02-05
 *  
 *  This is a simple generic dynanic hashmap implementation in C.
 *  It uses separate chaining to handle collisions.
 *  It resize automatically when the load balance is too high or too low (const in hashmap.c)
 *
 *  ---------- Features ---------
 *  - Generic : can store any type of key-value pairs
 *  - Dynamic : the hashmap will automatically grow and shrink when the load balance is too high or too low
 *  - Customizable : you can provide custom functions to handle memory allocation, deallocation, comparison and hash functions
 *  - Print : you can print the hashmap with custom print functions
 *  - Destroy : you can destroy the hashmap and all the key-value pairs with custom destroy functions
 *  
 *  -------- Limitations --------
 *  I would not recommend using this hashmap for large datasets, as it is not optimized for speed.
 *  Cause it uses a simple linked list to handle collisions, it is not the best choice for large datasets.
 *  Memory usage is also not optimized, as it malloc each key and value. (data is fragmented in memory)
 *  (if you want you can provide custom functions to handle memory allocation and deallocation, but it will not solve the "by design" problem)
 *  
 *  The hashmap will automatically grow and shrink when the load balance is too high or too low.
 *  The load balance is the number of key-value pairs divided by the capacity.
 *  
 *  The hashmap uses a hash function to distribute the keys in the table.
 *  We provide 2 hash functions (source: http://www.cse.yorku.ca/~oz/hash.html):
 *    - djb2 (default)
 *    - sdbm
*/

#ifndef __HASHMAP_H__
#define __HASHMAP_H__

#include <stdbool.h>

typedef unsigned long size_t;
typedef struct _hashmap_t hashmap_t;

#define HASHMAP_DEFAULT_CAPACITY 16
#define HASH_FUNC_DJB2 hashmap_hash_func_djb2
#define HASH_FUNC_SDBM hashmap_hash_func_sdbm
#define HASH_FUNC_ID hashmap_hash_func_id
#define HASH_FUNC_DEFAULT HASH_FUNC_DJB2

typedef size_t (*hash_fn_t)(const void* key, const size_t size);
typedef void (*print_fn_t)(const void *element);
typedef void (*destroy_fn_t)(void *element);
typedef void* (*alloc_copy_fn_t)(const void *element, const size_t size);
typedef int (*compare_fn_t)(const void *a, const void *b, const size_t size);

/// @brief Create a new hashmap
/// @param initial_capacity The initial capacity of the hashmap 
/// @param hash_fn The hash function to use
/// @param key_size The size of the key in bytes
/// @param value_size The size of the value in bytes
/// @return A pointer to the hashmap or NULL if an error occured
///
/// @note The key_size and value_size must be greater than 0 (asserted)
/// @note The hash_fn can be NULL, in this case, the hashmap will use the default hash function (djb2)
/// @note if initial_capacity is less than minimal_capacity, it will be set to minimal_capacity
/// 
/// @note If You want to provide custom functions, use the following functions:
/// @see hashmap_set_key_alloc_copy_fn : to provide custom copy functions for keys [DEFAULT: malloc+mempcy]
/// @see hashmap_set_value_alloc_copy_fn : to provide custom copy functions for values [DEFAULT: malloc+mempcy]
/// @see hashmap_set_key_destroy_fn : to provide custom destroy functions for keys. [DEFAULT: free]
/// @see hashmap_set_value_destroy_fn : to provide custom destroy functions for values. [DEFAULT: free]
/// @see hashmap_set_compare_fn : to provide a custom compare function (comparing keys) [DEFAULT: memcmp]
///
/// @note The hashmap will automatically grow and shrink when the load balance is too high or too low
/// @note The load balance is the number of key-value pairs divided by the capacity
///
/// @note for better results, use a hash function that returns a good distribution of values
/// @see HASH_FUNC_DJB2 - default hash function (if NULL is provided)
/// @see HASH_FUNC_SDBM
/// @see HASH_FUNC_ID (for unique ids)
hashmap_t* hashmap_create(size_t initial_capacity, hash_fn_t hash_fn, 
                          const size_t key_size, const size_t value_size);

/// @brief Destroy the hashmap
/// @param hm The hashmap to destroy
/// @note This will free all the key-value pairs using provided destroy functions
/// @see hashmap_set_key_destroy_fn
/// @see hashmap_set_value_destroy_fn
void hashmap_destroy(hashmap_t *hm);

/// @brief Get the value associated with the key
/// @param hm The hashmap
/// @param key The key to search for
/// @return A pointer to the value or NULL if the key was not found
/// @complexity ~O(1) -> O(n) where n is the number of same hash keys
void* hashmap_get(hashmap_t *hm, const void* key);

/// @brief Add a new key-value pair to the hashmap
/// @param hm The hashmap
/// @param key The key to add
/// @param value The value to add
/// @return A pointer to the added value, a pointer to the existing value or NULL if an error occured
/// @note If the key already exists, it will NOT REPLACE the old value ! (but return the old value)
/// @complexity O(1)
void* hashmap_add(hashmap_t *hm, const void* key, const void* value);

/// @brief Remove a key-value pair from the hashmap
/// @param hm The hashmap
/// @param key The key to remove
/// @return true if the key was removed, false otherwise (not found)
/// @note AFTER removing the key, the hashmap will automatically shrink if the load balance is too low
/// @complexity ~O(1) -> O(n) where n is the number of same hash keys
bool hashmap_remove(hashmap_t *hm, const void *key);

/// @brief Print the hashmap : some informations about the hashmap and all the key-value pairs
/// @param hm The hashmap
/// @param print_key_fn The function to print the key
/// @param print_value_fn The function to print the value
/// @note For better results, print on single line without newline character at the end
void hashmap_print(hashmap_t *hm, print_fn_t print_key_fn, print_fn_t print_value_fn);

/// @brief Get the number of key-value pairs in the hashmap
/// @param hm The hashmap
/// @return The number of key-value pairs
/// @complexity O(1)
/// @note this is the total amount of key-value pairs, not the number of chains
size_t hashmap_count(hashmap_t *hm);

/// @brief Get the capacity of the hashmap
/// @param hm The hashmap
/// @return The capacity of the hashmap
/// @complexity O(1)
size_t hashmap_capacity(hashmap_t *hm);

/// @brief Set the function to allocate and copy keys [DEFAULT: malloc+memcpy]
/// @param hm The hashmap
/// @param key_alloc_fn The function to allocate and copy keys
/// @note The function must return a pointer to the new key
void hashmap_set_key_alloc_copy_fn(hashmap_t *hm, alloc_copy_fn_t key_alloc_fn);

/// @brief Set the function to allocate and copy values [DEFAULT: malloc+memcpy]
/// @param hm The hashmap
/// @param value_alloc_fn The function to allocate and copy values
/// @note The function must return a pointer to the new value
void hashmap_set_value_alloc_copy_fn(hashmap_t *hm, alloc_copy_fn_t value_alloc_fn);

/// @brief Set the function to destroy keys [DEFAULT: free]
/// @param hm The hashmap
/// @param key_destroy_fn The function to destroy keys
/// @note The function must free the key
/// @note This function will be called when removing a key-value pair or when destroying the hashmap
void hashmap_set_key_destroy_fn(hashmap_t *hm, destroy_fn_t key_destroy_fn);

/// @brief Set the function to destroy values [DEFAULT: free]
/// @param hm The hashmap
/// @param value_destroy_fn The function to destroy values
/// @note The function must free the value
/// @note This function will be called when removing a key-value pair or when destroying the hashmap
void hashmap_set_value_destroy_fn(hashmap_t *hm, destroy_fn_t value_destroy_fn);

/// @brief Set the function to compare keys [DEFAULT: memcmp]
/// @param hm The hashmap
/// @param compare_fn The function to compare keys
/// @note The function must use standard compare function return values:
///       0 : equal
///      -1 : a < b
///       1 : a > b
void hashmap_set_compare_fn(hashmap_t *hm, compare_fn_t compare_fn);

/*--------------------------------- HASH FUNCTIONS ---------------------------------*/
/*
*   The hashmap uses a hash function to distribute the keys in the table.
*   We provide 2 hash functions (source: http://www.cse.yorku.ca/~oz/hash.html):
*       - djb2
*       - sdbm
*   
*   You can use them with the following macros (function pointers to pass to hashmap_create):
*       - HASH_FUNC_DJB2
*       - HASH_FUNC_SDBM
*
*   You can also use the HASH_FUNC_ID macro to use the hashmap with unique ids (size_t)
*   It is not an actual hash function, but it is useful when you want to use the hashmap with unique ids
*/

/// @brief The djb2 hash function
/// this algorithm (k=33) was first reported by dan bernstein many years ago in comp.lang.c.
/// @source [Mckenzie et al. Selecting a Hashing Algorithm, SP&E 20(2):209-224, Feb 1990]
size_t hashmap_hash_func_djb2(const void* key, const size_t size);

/// @brief The sdbm hash function
/// this algorithm was created for sdbm (a public-domain reimplementation of ndbm) database library.
size_t hashmap_hash_func_sdbm(const void* key, const size_t size);

/// @brief Function that satisfies the hash_fn_t type for ids (SIZE_T ONLY)
/// It is not an actual hash function, but it is useful when you want to use the hashmap with unique ids.
/// Du to its cast to size_t, it will ONLY WORK WITH SIZE_T keys ! Otherwise, undefined behavior :)
size_t hashmap_hash_func_id(const void* key, const size_t size);

#endif
