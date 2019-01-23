//
//  hash_table.h
//  hash_table
//
//  Created by Arjang Talattof on 21/01/2019.
//  Copyright © 2019 Univerisity of Michigan. All rights reserved.
//

#ifndef HASH_TABLE_H_
#define HASH_TABLE_H_

// Key-value pairs (items) stored in a struct.
typedef struct ht_item {
    char* key;
    char* value;
} ht_item;

// Hash table stores an array of pointers to
// items, and some details about its size and
// how full it is.
typedef struct {
    int size_index;
    int size;
    int count;
    ht_item** items;
} ht_hash_table;

// Hash table API
ht_hash_table* ht_new();
void ht_del_hash_table(ht_hash_table* ht);
void ht_insert(ht_hash_table* ht, const char* key, const char* value);
char* ht_search(ht_hash_table* ht, const char* key);
void ht_delete(ht_hash_table* h, const char* key);
static int ht_generic_hash(const char* s, const int a, const int m);
static int ht_hash(const char* s, const int num_buckets, const int attempt);

#endif  // HASH_TABLE_H_
