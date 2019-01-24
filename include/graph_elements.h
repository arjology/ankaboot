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
typedef struct {
    char* node;
    float* distance;
} neighbour;

typedef struct {
    neighbour** neighbours;
    int size_index;
    int size;
    int count;
} neighbours;

// Vertex defined by a key
typedef struct {
    int size_index;
    int size;
    int count;
    char* node;
    neighbours** neighbours;
} edges_table;

// Vertex and properties
typedef struct {
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

// ------------------------------------------
// Graph API

// Edges
edges_table* create_edges();
void add_edge(edges_table* E, neighbour* n);
void delete_edges(edges_table* E);
neighbour** find_neighbours(edges_table* E, const char* key);
neighbours* create_neighbours(const int size_index);

//Nodes
nodes_table* create_nodes();
void add_node(nodes_table* N, node* n);
void delete_nodes(nodes_table* N);
node* find_node(nodes_table* N, const char* key);

graph* create_graph();
void delete_graph(graph* G);



#endif // GRAPH_ELEMENTS_H_
