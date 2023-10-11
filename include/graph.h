#ifndef LEPL1503__GRAPH__
#define LEPL1503__GRAPH__

#if defined(__APPLE__) || defined(__WIN32__) || defined(__arm__)
#define MY_FORMAT_SPECIFIER "%lld"
#else
#define MY_FORMAT_SPECIFIER "%ld"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>
#include "../include/portable_endian.h"
#include "../include/portable_semaphore.h"
#include <arpa/inet.h>

// Definition of the different structures used in the program
typedef struct branch{
    uint32_t * node_from;
    uint32_t * node_to;
    int32_t * cost;
} branch_t;

typedef struct file_data{
    uint32_t nb_edges;
    uint32_t nb_nodes;
} file_data_t;

typedef struct graph{
    file_data_t * file_infos; 
    branch_t * graph_data;
} graph_t;

typedef struct ford{
    int32_t * dist;
    int32_t * path;
} ford_t;

typedef struct mcost{
    int64_t cost;
    uint32_t node;
} mcost_t;

/*
This file contains all the functions used to read the file and to create the graph as well as a short description of each function.
For more information about the functions, please refer to the file graph.c
*/

/*
Function : int_to_big_endian
Description : This function converts big endian to uint32_t.
*/
uint32_t big_endian_to_int(unsigned char *buffer);

/*
Function : get_file_info
Description : This function reads the file and returns the number of nodes and edges as well as the graph stored in the file.
*/
graph_t * get_file_info(FILE * file);

/*
Function : free_graph_struct
Used to free the memory allocated for the graph structure.
*/
void free_graph_struct(graph_t* graph);

/*
Function : bellman_ford
Description : This function implements the Bellman-Ford algorithm. It returns the shortest path from the source to each node.
*/
ford_t* bellman_ford(uint32_t nb_nodes, uint32_t nb_edges, branch_t* links, uint32_t source, bool verbose);

/*
Function : free_ford_struct
Used to free the memory allocated for the ford structure.
*/
void free_ford_struct(ford_t * ford);

/*
Function : get_max
Description : This function returns the maximum cost of the shortest path from the source to each node.
*/
mcost_t * get_max(int32_t nb_nodes, int32_t * dist, uint32_t source);

/*
Function : free_max_struct
Used to free the memory allocated for the mcost structure.
*/
void free_max_struct(mcost_t * mcost);

/*
Function : get_path
Description : This function returns the shortest path from the source to the destination.
*/
int32_t * get_path(uint32_t dest, uint32_t source, int32_t* path, int32_t* size);

/*
Function : free_path
Used to free the memory allocated for the path.
*/
void free_path(int32_t * path);

/*
Function : write_to_file
Description : This function writes the results in the output file.
*/
int write_to_file(FILE * file, uint32_t source, mcost_t * max, int32_t size_path, int32_t * path);

#endif // LEPL1503__GRAPH__