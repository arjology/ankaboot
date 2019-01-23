//
//  graph_elements.c
//  hash_table
//
//  Created by Arjang Talattof on 22/01/2019.
//  Copyright © 2019 Univerisity of Michigan. All rights reserved.
//

#ifndef GRAPH_ELEMENTS_H_
#define GRAPH_ELEMENTS_H_

#include <stdlib.h>
#include "geography.h"

// Neighbor storign its key and its distance
typedef struct neighbour {
    char* node;
    float* distance;
} neighbour;

// Vertex defined by a key
typedef struct {
    int size_index;
    int size;
    int count;
    char* node;
    neighbour** neighbours;
} edges_table;

// Vertex and properties
typedef struct node {
    char* key;
    gps* location;
} node;

// Dynamically allocated array of vertices
typedef struct {
    int size_index;
    int size;
    int count;
    node** nodes;
} nodes_table;

typedef struct {
    nodes_table N;
    edges_table E;
} graph;

// Graph API
edges_table* create_edges();
void add_edge(edges_table* E, const neighbour* neighbour);
void delete_edges(edges_table* E);

nodes_table* create_nodes();
void add_node(nodes_table* N, const node* node);
void delete_nodes(nodes_table* N);

graph* create_graph();
void delete_graph(graph* G);
char* find_node(graph* G, const char* node);
neighbour** find_neighbours(graph* G, const char* node);


#endif // GRAPH_ELEMENTS_H_
