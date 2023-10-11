#include "../include/graph.h"

/*
Function big_endian_to_int
-------------------------------------

Function that converts big endian to uint32_t

Input:
----------
unsigned char *buffer : the buffer containing the bits

Output:
----------
uint32_t : the converted value
*/
uint32_t big_endian_to_int(unsigned char *buffer) {
    return ((uint32_t)buffer[0] << 24) | ((uint32_t)buffer[1] << 16) | ((uint32_t)buffer[2] << 8) | buffer[3];
}

/*
Function get_file_info
-------------------------------------

Function that returns the pointer to the graph_t structure of the graph that will be worked on
This structure contains, the number of edges and nodes that the graph but also the branches that link the nodes

Input:
----------
char* file_name : the location/name of the file

Output:
----------
graph_t * graph = the mentionned structure
NULL + perror message if there was an error
*/
graph_t * get_file_info(FILE * file){ 
    
    // Check if file is not null
    if (file == NULL) {perror("File is null");return NULL;}
    // Create the structure in which the graph will be stored + checks
    graph_t* graph = malloc(sizeof(graph_t));
    if (graph == NULL) {fclose(file);perror("Could not allocate memory for the graph, free memory and try again");return NULL;};

    // Check if the file is empty
    fseek(file, 0, SEEK_END);
    if (ftell(file) == 0) {
        perror("File is empty");
        free(graph);
        fclose(file);
        return NULL;}
    rewind(file);

    // Initialize the structure for the basic info
    graph->file_infos = malloc(sizeof(file_data_t));
    if (graph->file_infos == NULL) {fclose(file);free(graph->graph_data);free(graph);
        perror("Could not allocate memory for file_infos, free memory and try again");return NULL;}
    
    /* Reading basic info */ 

    unsigned char buffer[4]; // Buffer for the bits in the file + check
    if (fread(buffer, 1, 4, file) != 4) {fclose(file); return NULL;} 
    // Big-edian -> host
    int32_t nb_nodes = big_endian_to_int(buffer);
    if (fread(buffer, 1, 4, file) != 4) {fclose(file); return NULL;}
    int32_t nb_edges = big_endian_to_int(buffer);

    /* If the graph is not correctly formated we do not need to contiue */
    if (nb_nodes <= 1 || nb_edges <= 0){perror("Mal structuré");fclose(file); free(graph->file_infos);free(graph); return NULL;}
    //------------------------------------------------------
    
    /* Reading basic info done */ 

    /* Reading the graph branches */
    
    // Check if the file is corrupted
    fseek(file, 0, SEEK_END);
    int32_t size = ftell(file);
    if (size != (nb_edges*12)+8) {perror("Corrompu");fclose(file); free(graph->file_infos);free(graph); return NULL;}
    
    // If not we continue and allocate memory for the graph data
    fseek(file, 8, SEEK_SET);
    graph->graph_data = (branch_t*)malloc(nb_edges * sizeof(branch_t));
    if (graph->graph_data == NULL) {fclose(file);free(graph);
        perror("Could not allocate memory for graph_data, free memory and try again");return NULL;}
    
    graph->graph_data->cost = (int32_t*)malloc(nb_edges * sizeof(int32_t));
    if (graph->graph_data->cost == NULL) {fclose(file);free(graph);
        perror("Could not allocate memory for graph_data structure, free memory and try again");return NULL;}

    graph->graph_data->node_from = (uint32_t*)malloc(nb_edges * sizeof(uint32_t));
    if (graph->graph_data->node_from == NULL) {fclose(file);free(graph);
        perror("Could not allocate memory for graph_data structure, free memory and try again");return NULL;}
    
    graph->graph_data->node_to = (uint32_t*)malloc(nb_edges * sizeof(uint32_t));
    if (graph->graph_data->node_to == NULL) {fclose(file);free(graph);
        perror("Could not allocate memory for graph_data structure, free memory and try again");return NULL;}
    
    // We read the file and store the data in the graph structure

    for (int i = 0; i < nb_edges; i++){
        unsigned char buffer1[4]; // Buffer + checks for branches
        if (fread(buffer1, 1, 4, file) != 4) {fclose(file);free(graph->graph_data); free(graph->file_infos);free(graph);return NULL;}
        graph->graph_data->node_from[i] = big_endian_to_int(buffer1);
        
        // Check if it is a valid node
        if (graph->graph_data->node_from[i] < 0 || graph->graph_data->node_from[i] >= nb_nodes) {perror("Wrong format of the input file");fclose(file);free(graph->graph_data); free(graph->file_infos);free(graph);return NULL;}
        
        if (fread(buffer1, 1, 4, file) != 4) {fclose(file);free(graph->graph_data); free(graph->file_infos);free(graph);return NULL;}
        graph->graph_data->node_to[i] = big_endian_to_int(buffer1);
        
        // Check if it is a valid node
        if (graph->graph_data->node_to[i] < 0 || graph->graph_data->node_to[i] >= nb_nodes) {perror("Wrong format of the input file");fclose(file);free(graph->graph_data); free(graph->file_infos);free(graph);return NULL;}
        
        if (fread(buffer1, 1, 4, file) != 4) {fclose(file);free(graph->graph_data); free(graph->file_infos);free(graph);return NULL;}
        graph->graph_data->cost[i] = big_endian_to_int(buffer1);
        
    }
    fclose(file);
    graph->file_infos->nb_nodes = nb_nodes;
    graph->file_infos->nb_edges = nb_edges;
    return graph;
}

/*
Function free_graph_struct
-------------------------------------

Function used to free the memory allocated to the graph structure that was used
*/
void free_graph_struct(graph_t* graph){
    free(graph->file_infos);
    free(graph->graph_data->cost);
    free(graph->graph_data->node_from);
    free(graph->graph_data->node_to);
    free(graph->graph_data);
    free(graph);
}
//---------------------------------------------------------------------------------------------------------

/*
Function bellman_ford
-------------------------------------
This function implements the bellman ford algorithm, it finds the shortest path between 2 nodes in a graph.
This algorithm also works when negative values are introduced for the cost between 2 nodes.

Input:
----------
uint32_t nb_nodes : number of nodes in the graph
uint32_t nb_edges : number of edges in the graph
branch_t * links : array of size nb_edges containing the links of the graph
uint32_t s : source node
bool verbose : boolean to print the result or not

Output:
----------
ford_t * ford_links : structure that keeps track of distances between the source and each other node, the path between the source and the other nodes 
NULL + perror message if there was an error
*/
ford_t* bellman_ford(uint32_t nb_nodes, uint32_t nb_edges, branch_t * links, uint32_t source, bool verbose){
    // Initialization
    ford_t* ford_links = (ford_t*)malloc(sizeof(ford_t)); 
    if (ford_links == NULL){perror("Erreur d'allocation de mémoire pour la structure de bellman_ford"); return NULL;}
    int32_t* distances = (int32_t*)malloc(sizeof(int32_t) * nb_nodes);
    if (distances == NULL){perror("Erreur d'allocation de mémoire pour la structure de bellman_ford"); free(ford_links); return NULL;}
    int32_t* paths = (int32_t*)malloc(sizeof(int32_t) * nb_nodes);
    if (paths == NULL){perror("Erreur d'allocation de mémoire pour la structure de bellman_ford"); free(ford_links); free(distances); return NULL;}

    for (int i = 0; i < nb_nodes; i++){
        distances[i] = INT32_MAX;
        paths[i] = -1;
    }
    distances[source] = 0;
    
    // Main loop for belman ford
    for (int i = 0; i < nb_nodes-1; i++) { // Allows us to terminate the loop if we see that it has no reason to continue 
        for (int j = 0; j < nb_edges; j++) {
            uint32_t node_from = links->node_from[j];
            uint32_t node_to = links->node_to[j];
            int32_t cost = links->cost[j];
            if (distances[node_from] != INT32_MAX && distances[node_to] > distances[node_from] + cost) {
                distances[node_to] = distances[node_from] + cost;
                paths[node_to] = node_from;
                
            }
        }
    }
    // Detection of negative cycle
    for (int k = 0; k < nb_edges; k++){
        uint32_t node_from = links->node_from[k];
        uint32_t node_to = links->node_to[k];
        int32_t cost = links->cost[k];
        if (distances[node_from] != INT32_MAX && distances[node_to] > distances[node_from] + cost){
            if (verbose){
                printf("Negative cycle detected\n");
            }
            // Reset distancesances and paths because of the negative cycle
            for (int i = 0; i< nb_nodes; i++){
                distances[i] = INT32_MAX;
                paths[i] = -1;
            }
            distances[source] = 0;
        }
    }
    ford_links->dist = distances;
    ford_links->path = paths;
    return ford_links;
}

/*
Function free_ford_struct
-------------------------------------

Function used to free the memory allocated to the ford structure that was used
*/
void free_ford_struct(ford_t* ford){
    free(ford->dist);
    free(ford->path);
    free(ford);
}
//---------------------------------------------------------------------------------------------------------

/*
Function get_max
-------------------------------------
This function returns us the most "expensive node" to reach as a structure that stores the node and the cost

Input:
----------
int32_t nb_nodes : the number of nodes in the graph
int32_t * dist : pointer to the array that stores the distances from each node to each node
uint32_t soucre : the source node 

Output:
----------
mcost_t * max : the structure that stores the node and cost
NULL + perror message if there was an error
*/
mcost_t * get_max(int32_t nb_nodes, int32_t * dist, uint32_t source){

    //Code translated from python
    mcost_t * max = (mcost_t *)malloc(sizeof(mcost_t));
    if (max == NULL){perror("Could not allocate memory in the get_max function");return NULL;}
    int64_t max_cost = -INT64_MAX;
    uint32_t max_node = source;
    for (int node_idx = 0; node_idx < nb_nodes; node_idx++){
        if (node_idx != source && dist[node_idx] != INT32_MAX && dist[node_idx] >= max_cost){
            if (dist[node_idx] == max_cost && max_node < node_idx){
                continue;
            }
            max_node = node_idx;
            max_cost = dist[node_idx];
        }
    }
    if (max_node == source){max_cost = 0;}
    max->cost = max_cost;
    max->node = max_node;
    return max;
}

/*
Function free_max_struct
-------------------------------------

Function used to free the memory allocated to the mcost structure that was used
*/
void free_max_struct(mcost_t * mcost){
    free(mcost);
}
//---------------------------------------------------------------------------------------------------------

/*
Function get_path
-------------------------------------
This function returns the path from the source node to the destination node given the path array that is provided

Input:
----------
uint32_t dest : the destination node 
uint32_t source : the source node 
int32_t * path : pointer to the array that stores the paths from each node to each node
int32_t * size : pointer that keeps track of the lenght of the path  

Output:
----------
int32_t * the_path : the array with the path
NULL + perror message if there was an error
*/
int32_t* get_path(uint32_t dest, uint32_t source, int32_t* path, int32_t* size) {
    
    // Initialize the path array
    int32_t* the_path = (int32_t*) calloc(*size, sizeof(int32_t));
    if (the_path == NULL) {
        perror("Could not allocate memory in the get_path function");
        return NULL;
    }

    uint32_t i = dest;
    uint32_t the_path_indx = 0;

    // Create the path array starting from the destination back to the source
    while (i != source) {
        the_path[the_path_indx++] = i;
        i = path[i];
    }

    // Add the source node to the path as the last element + get the size of the path
    the_path[the_path_indx++] = source;
    *size = the_path_indx;

    // Reverse the array to return the path to take in the right order, source -> destination 
    for (uint32_t j = 0; j < the_path_indx / 2; j++) {
        uint32_t temp = the_path[j];
        the_path[j] = the_path[the_path_indx - j - 1];
        the_path[the_path_indx - j - 1] = temp;
    }

    return the_path;
}

/*
Function free_path
-------------------------------------

Function used to free the memory allocated to the path array
*/
void free_path(int32_t * path){
    free(path);
}
//---------------------------------------------------------------------------------------------------------

/*
Function write_to_file
-------------------------------------
This function writes the data to the file in big edian binary format

Input:
----------
FILE * file : the file pointer
file_data_t * file_data : the structure that stores the data from the file
mcost_t * max : the structure that stores the node and cost
int32_t * size_path : pointer that keeps track of the lenght of the path
int32_t * path : the array with the path

Output:
----------
0 if everything went well
1 if there was an error
*/
int write_to_file(FILE * file, uint32_t source, mcost_t * max, int32_t size_path, int32_t * path) {
    
    // Convert source node to big endian format
    uint32_t source_be = htonl(source);
    
    // Convert destination node to big endian format
    uint32_t dest_be = htonl(max->node);
    
    // Convert cost to big endian format
    int64_t cost_ho;
    memcpy(&cost_ho, &(max->cost), sizeof(double));
    uint64_t cost_be = htobe64(cost_ho);
    
    // Convert size of path to big endian format
    int32_t size_be = htonl(size_path);

    // Write source node to file
    if (fwrite(&source_be, sizeof(source_be), 1, file) != 1){
        perror("Could not write to file");
        return 1;
    }
    
    // Write destination node to file
    if (fwrite(&dest_be, sizeof(dest_be), 1, file) != 1){
        perror("Could not write to file");
        return 1;
    }
    
    // Write cost to file
    if (fwrite(&cost_be, sizeof(cost_be), 1, file) != 1){
        perror("Could not write to file");
        return 1;
    }
    
    // Write size of path to file
    if (fwrite(&size_be, sizeof(size_be), 1, file) != 1){
        perror("Could not write to file");
        return 1;
    }
    
    // Convert path nodes to big endian format and write to file
    for(int i = 0; i < size_path; i++){
        uint32_t node_be = htonl(path[i]);
        if (fwrite(&node_be, sizeof(node_be), 1, file) != 1){
            perror("Could not write to file");
            return 1; 
        }
    }
    
    return 0; // Everything went well
}