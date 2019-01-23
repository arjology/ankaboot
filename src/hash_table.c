//
//  hash_table.c
//  hash_table
//
//  Created by Arjang Talattof on 21/01/2019.
//  Copyright © 2019 Univerisity of Michigan. All rights reserved.
//

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "xmalloc.h"

#include "hash_table.h"
#include "prime.h"

// HT_DELETED_ITEM is used to mark a bucket containing a deleted item
static ht_item HT_DELETED_ITEM = {NULL, NULL};

// HT_PRIMEs are parameters in the hashing algorithm
static const int HT_PRIME_1 = 151;
static const int HT_PRIME_2 = 163;
static const int HT_INITIAL_BASE_SIZE = 0;

// Define initialization functions for `ht_item`s.
// This function allocates a chunk of memory the size
// of an `ht_item`, and saves a copy of the strings
// `k` and `v` in the new chunk of memory. The
// function is marked as `static` because it will
// only ever be called by code internal to the
// hash table.
static ht_item* ht_new_item(const char* k, const char* v) {
    ht_item* i = malloc(sizeof(ht_item));
    i->key = strdup(k);
    i->value = strdup(v);
    return i;
}

// `ht_new` initializes a new hash table.
// `size` defines how many items we can store,
// fixed at 53 for now. This will be later expanded
// in the section on resizing. Initialize the array
// of items with `calloc`, which fills the allocated
// memory with `NULL` bytes. A `NULL` entry in the
// array indicates that the bucket is empty.
// Support creating a hash table of a certain size. To do this,
// `ht_new_sized` is called by `ht_new`.
static ht_hash_table* ht_new_sized(const int size_index) {
    ht_hash_table* ht = xmalloc(sizeof(ht_hash_table));
    ht->size_index = size_index;
    
    const int base_size = 50 << ht->size_index;
    ht->size = next_prime(base_size);
    
    ht->count = 0;
    ht->items = xcalloc((size_t)ht->size, sizeof(ht_item*));
    return ht;
}

ht_hash_table* ht_new() {
    return ht_new_sized(HT_INITIAL_BASE_SIZE);
}

// Resize:
// Ensure size of hash table is not being resized below its minimum.
// Initialize new hash table with desired size. All non-`NULL` or
// deleted items are inserted into the new hash table. Then swap
// attributes of the new and old hash tables before deleting the latter.
static void ht_resize(ht_hash_table* ht, const int direction) {
    const int new_size_index = ht->size_index + direction;
    if (new_size_index < HT_INITIAL_BASE_SIZE) {
        // Don't resize down the smallest hash table
        return;
    }
    // Create a temporary new hash table to insert items into
    ht_hash_table* new_ht = ht_new_sized(new_size_index);
    // Iterate through existing hash table, add all items to new
    for (int i = 0; i < ht->size; i++) {
        ht_item* item = ht->items[i];
        if (item != NULL && item != &HT_DELETED_ITEM) {
            ht_insert(new_ht, item->key, item->value);
        }
    }
    
    // Pass new_ht and ht's properties. Delete new_ht
    ht->size_index = new_ht->size_index;
    ht->count = new_ht->count;
    
    // To delete new_ht, we give it ht's size and items
    const int tmp_size = ht->size;
    ht->size = new_ht->size;
    new_ht->size = tmp_size;
    
    ht_item** tmp_items = ht->items;
    ht->items = new_ht->items;
    new_ht->items = tmp_items;
    
    ht_del_hash_table(new_ht);
}

// Resizing up and down
static void ht_resize_up(ht_hash_table* ht) {
    ht_resize(ht, 1);
}
static void ht_resize_down(ht_hash_table* ht) {
    ht_resize(ht, -1);
}

// Functions for deleting `ht_item`s and `ht_hash_table`s
// which `free` the memory allocated, preventing
// memory leaks.
static void ht_del_item(ht_item* i) {
    free(i->key);
    free(i->value);
    free(i);
}

void ht_del_hash_table(ht_hash_table* ht) {
    for (int i = 0; i < ht->size; i++) {
        ht_item* item = ht->items[i];
        if (item != NULL && item != &HT_DELETED_ITEM) {
            ht_del_item(item);
        }
    }
    free(ht->items);
    free(ht);
}

// Implementation of a hash function.
// Takes a string as input and returns a number between
// `0` and `m`, the desired bucket array length.
// Returns an even distribution of bucket indexes for
// an average set of inputs. If the hash function is
// unevenly distributed, it will put more items in some
// buckets than others which leads to a higher rate of
// collisions, thereby reducing the efficiency of the
// hash table.
static int ht_generic_hash(const char* s, const int a, const int m) {
    long hash = 0;
    const int len_s = strlen(s);
    for (int i = 0; i < len_s; i++) {
        /* Map char to a large integer */
        hash += (long)pow(a, len_s - (i+1)) * s[i];
        hash = hash % m;
    }
    return (int)hash;
}

// Handling collisions.
// Mapping an infinitely large number of inputs to a
// finite number of outputs. Different inputs will
// map to the same array index, causing bucket
// collisions, something that must be dealt with.
// Here, open addressing with double hashing makes
// use of two hash functions to calculate the index an
// item should be stored at after `i` collisions.
static int ht_hash(const char* s, const int num_buckets, const int attempt) {
    const int hash_a = ht_generic_hash(s, HT_PRIME_1, num_buckets);
    const int hash_b = ht_generic_hash(s, HT_PRIME_2, num_buckets);
    return (hash_a + (attempt * (hash_b + 1))) % num_buckets;
}

// Insertion of a new key-value pair:
// Iterate through indexes until an empty bucket is
// found, where the item will be inserted and the hash
// table's `count` attribute incremented to indicate
// insertion of a new item. This is useful for
// resizing. If encountering a deleted item, the new node
// can be inserted in its place. If two items are inserted into the
// same key, the keys will collide and the second item will be inserted
// into the next available bucket. When searching for the key, the
// original key will always be found and the second item will be
// inaccessible. To handle this, the previous item can be deleted
// and the new item inserted in its place.
// To perform resizing, check load on hash table during inserts and deletes.
void ht_insert(ht_hash_table* ht, const char* key, const char* value) {
    const int load = ht->count * 100 / ht->size;
    if (load > 70) {
        ht_resize_up(ht);
    }
    ht_item* item = ht_new_item(key, value);
    int index = ht_hash(item->key, ht->size, 0);
    ht_item* cur_item = ht->items[index];
    int i = 1;
    while(cur_item != NULL) {
        if (cur_item != &HT_DELETED_ITEM) {
            if (strcmp(cur_item->key, key) == 0) {
                ht_del_item(cur_item);
                ht->items[index] = item;
                return;
            }
        }
        index = ht_hash(item->key, ht->size, i);
        cur_item = ht->items[index];
        i++;
    }
    ht->items[index] = item;
    ht->count++;
}

// Searching for keys:
// At iteration of the `while` loop check whether the item's key matches
// the key of interest and return the value if found. If the `while` loop
// reaches a `NULL` value then return `NULL` indicating that the item
// was not found. Ignore and jump over item marked as deleted.
char* ht_search(ht_hash_table* ht, const char* key) {
    int index = ht_hash(key, ht->size, 0);
    ht_item* item = ht->items[index];
    int i = 1;
    while (item != NULL) {
        if (item != &HT_DELETED_ITEM) {
            if (strcmp(item->key, key) == 0) {
                return item->value;
            }
        }
        index = ht_hash(key, ht->size, i);
        item = ht->items[index];
        i++;
    }
    return NULL;
}

// Deleting key:
// The item may be part of a collision chain, thus removing it
// would break the chain and make finding items in the tail impossible.
// Instead of deleting, the item is marekd as deleted by replacing it with
// a pointer to a global sentinel item which represents that a bucket
// contains a deleted item. After deleting, the hash table `count` value
// is decremented.
// To perform resizing, check load on hash table during inserts and deletes.
void ht_delete(ht_hash_table* ht, const char* key) {
    const int load = ht->count * 100 / ht->size;
    if (load < 10) {
        ht_resize_down(ht);
    }
    int index = ht_hash(key, ht->size, 0);
    ht_item* item = ht->items[index];
    int i = 1;
    while (item != NULL && item != &HT_DELETED_ITEM) {
        if (strcmp(item->key, key) == 0) {
            ht_del_item(item);
            ht->items[index] = &HT_DELETED_ITEM;
        }
        index = ht_hash(key, ht->size, i);
        item = ht->items[index];
        i++;
    }
    ht->count--;
}
