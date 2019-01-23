//
//  graph_elements.c
//  hash_table
//
//  Created by Arjang Talattof on 22/01/2019.
//  Copyright © 2019 Univerisity of Michigan. All rights reserved.
//

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "xmalloc.h"

#include "graph_elements.h"
#include "hash_table.h"
#include "prime.h"

// HT_DELETED_ITEM is used to mark a bucket containing a deleted item
static neighbour DELETED_NEIGHBOUR = {NULL, NULL};
static node DELETED_NODE = {NULL, NULL};

// PRIMEs are parameters in the hashing algorithm
static const int PRIME_1 = 151;
static const int PRIME_2 = 163;
static const int INITIAL_BASE_SIZE = 0;

// Define initialization functions for `node`s and `neighbour`s.
// This function allocates a chunk of memory the size
// of an `neighbour`, and saves a copy of the `key` and
// float `distance`for neighbour, and `key` and `location` for the node,
// in the new chunk of memory. The function is marked as
// `static` because it will only ever be called by code internal to the
// graph.

static node* new_node(const char* key, gps* location) {
    node* n = malloc(sizeof(node));
    n->key = strdup(key);
    n->location = location;
    return n;
}

static neighbour* new_neighbour(const char* node, float* distance) {
    neighbour* n = malloc(sizeof(neighbour));
    n->node = strdup(node);
    n->distance = distance;
    return n;
}

// `create_nodes` and `create_edge` initializes a new hash table.
// `size` defines how many nodes and edges we can store for each,
// fixed at 53 for now. Initialize the array of nodes and neighbours
// with `calloc`, which fills the allocated memory with `NULL`
// bytes. A `NULL` entry in the array indicates that the bucket is empty.
// Support creating a hash table of a certain size. To do this,
// `create_edges_sized` and `create_nodes_sized` are called by `create_edges`
// and `create_nodes`, respectively.
static edges_table* create_edges_sized(const int size_index) {
    edges_table* E = xmalloc(sizeof(edges_table));
    E->size_index = size_index;
    
    const int base_size = 50 << E->size_index;
    E->size = next_prime(base_size);
    
    E->count = 0;
    E->neighbours = xcalloc((size_t)E->size, sizeof(neighbour*));
    return E;
}

edges_table* create_edges() {
    return create_edges_sized(INITIAL_BASE_SIZE);
}

static nodes_table* create_nodes_sized(const int size_index) {
    nodes_table* N = xmalloc(sizeof(nodes_table));
    N->size_index = size_index;
    
    const int base_size = 50 << N->size_index;
    N->size = next_prime(base_size);
    
    N->count = 0;
    N->nodes = xcalloc((size_t)N->size, sizeof(node*));
    return N;
}

nodes_table* create_nodes() {
    return create_nodes_sized(INITIAL_BASE_SIZE);
}

// Create a graph from the nodes and edges tables.
graph* create_graph() {
    edges_table* E = create_edges();
    nodes_table* N = create_nodes();
    graph* G = xmalloc(sizeof(graph));
    G->N = *N;
    G->E = *E;
    return G;
}

// Resize:
// Ensure size of edges or nodes table is not being resized below its minimum.
// Initialize new hash table with desired size. All non-`NULL` or
// deleted items are inserted into the new hash table. Then swap
// attributes of the new and old hash tables before deleting the latter.
static void resize_nodes(nodes_table* N, const int direction) {
    const int new_size_index = N->size_index + direction;
    if (new_size_index < INITIAL_BASE_SIZE) {
        // Don't resize down the smallest hash table
        return;
    }
    // Create a temporary new hash table to insert items into
    nodes_table* new_N = create_nodes_sized(new_size_index);
    // Iterate through existing hash table, add all items to new
    for (int i = 0; i < N->size; i++) {
        node* node = N->nodes[i];
        if (node != &DELETED_NODE) {
            add_node(N, node);
        }
    }
    
    // Pass new_ht and ht's properties. Delete new_ht
    N->size_index = new_N->size_index;
    N->count = new_N->count;
    
    // To delete new_ht, we give it ht's size and items
    const int tmp_size = N->size;
    N->size = new_N->size;
    new_N->size = tmp_size;
    
    node** tmp_nodes = N->nodes;
    N->nodes = new_N->nodes;
    new_N->nodes = tmp_nodes;
    
    delete_nodes(new_N);
}

static void resize_edges(edges_table* E, const int direction) {
    const int new_size_index = E->size_index + direction;
    if (new_size_index < INITIAL_BASE_SIZE) {
        // Don't resize down the smallest hash table
        return;
    }
    // Create a temporary new hash table to insert items into
    edges_table* new_E = create_edges_sized(new_size_index);
    // Iterate through existing hash table, add all items to new
    for (int i = 0; i < E->size; i++) {
        neighbour* neighbour = E->neighbours[i];
        if (neighbour != NULL && neighbour != &DELETED_NEIGHBOUR) {
            add_edge(E, neighbour);
        }
    }
    
    // Pass new_ht and ht's properties. Delete new_ht
    E->size_index = new_E->size_index;
    E->count = new_E->count;
    
    // To delete new_ht, we give it ht's size and items
    const int tmp_size = E->size;
    E->size = new_E->size;
    new_E->size = tmp_size;
    
    neighbour** tmp_neighbours = E->neighbours;
    E->neighbours = new_E->neighbours;
    new_E->neighbours = tmp_neighbours;
    
    delete_edges(new_E);
}

// Deleting neighbour and edges table
static void delete_neighbour(neighbour* n) {
    free(n->node);
    free(n->distance);
    free(n);
}

static void delete_node(node* n) {
    gps* location = n->location;
    free(location->lat);
    free(location->lon);
    free(n->location);
    free(n->key);
    free(n);
}

void delete_edges(edges_table* E) {
    for (int i = 0; i < E->size; i++) {
        neighbour* n = E->neighbours[i];
        if (n != NULL && n != &DELETED_NEIGHBOUR) {
            delete_neighbour(n);
        }
    }
    free(E->neighbours);
    free(E->node);
    free(E);
}

void delete_nodes(nodes_table* N) {
    for (int i = 0; i < N->size; i++) {
        node* n = N->nodes[i];
        if (n != NULL && n != &DELETED_NODE) {
            delete_node(n);
        }
    }
    free(N->nodes);
    free(N);
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
void add_edge(edges_table* E, neighbour* n) {
    const int load = E->count * 100 / E->size;
    if (load > 70) {
        resize_edges(E, 1);
    }
    int index = ht_hash(n->node, E->size, 0);
    neighbour* cur_neighbour = E->neighbours[index];
    int i = 1;
    while(cur_neighbour != NULL) {
        if (cur_neighbour != &DELETED_NEIGHBOUR) {
            if (strcmp(cur_neighbour->node, n->node) == 0) {
                delete_neighbour(cur_neighbour);
                E->neighbours[index] = n;
                return;
            }
        }
        index = ht_hash(n->node, E->size, i);
        cur_neighbour = E->neighbours[index];
        i++;
    }
    E->neighbours[index] = n;
    E->count++;
}

void add_node(nodes_table* N, node* n) {
    const int load = N->count * 100 / N->size;
    if (load > 70) {
        resize_nodes(N, 1);
    }
    int index = ht_hash(n->key, N->size, 0);
    node* cur_node = N->nodes[index];
    int i = 1;
    while(cur_node != NULL) {
        if (cur_node != &DELETED_NODE) {
            if (strcmp(cur_node->key, n->key) == 0) {
                delete_node(cur_node);
                N->nodes[index] = n;
                return;
            }
        }
        index = ht_hash(n->key, N->size, i);
        cur_node = N->nodes[index];
        i++;
    }
    N->nodes[index] = n;
    N->count++;
}

// Searching for keys:
// At iteration of the `while` loop check whether the item's key matches
// the key of interest and return the value if found. If the `while` loop
// reaches a `NULL` value then return `NULL` indicating that the item
// was not found. Ignore and jump over item marked as deleted.
node* find_node(nodes_table* N, const char* key) {
    int index = ht_hash(key, N->size, 0);
    node* n = N->nodes[index];
    int i = 1;
    while (n != NULL) {
        if (n != &DELETED_NODE) {
            if (strcmp(n->key, key) == 0) {
                return n;
            }
        }
        index = ht_hash(key, N->size, i);
        n = N->nodes[index];
        i++;
    }
    return NULL;
}

neighbour** find_neighbours(edges_table* E, const char* key) {
    int index = ht_hash(key, N->size, 0);
    neighbour** n = E->neighbours[index];
}
