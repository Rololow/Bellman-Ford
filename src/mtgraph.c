#include "../include/graph.h"
#include "../include/mtgraph.h"

uint32_t BUFFERSIZE;
uint32_t NTHREADS;

// Declaration of shared buffers and everything related to them
pthread_mutex_t written_nodes_mutex;
pthread_mutex_t counter_mutex;
pthread_mutex_t dispatch_mutex;
pthread_mutex_t write_mutex;

uint32_t *dispatchbuffer;
thread_data_t *writebuffer;

int dispatchhead = 0;
int dispatchtail = 0;
int writehead = 0;
int writetail = 0;

pthread_cond_t dispatch_not_empty;
pthread_cond_t dispatch_not_full;
pthread_cond_t write_not_empty;
pthread_cond_t write_not_full;

// Declaration of global varaibles that store the graph and the general purpose variables
// When created and filled, graph will be free to access for all threads (no need to lock it since it is read only)
graph_t *graph; 
bool verbose = false;
bool show = false;
bool error = false;
int written_nodes = 0;
int computed_nodes = 0;

/*
Function readthread 
---------------------------

Function executed by the first threadn, it dispatches the source nodes to the other threads

Input:
----------
None

Output:
----------
None
*/
void *readthread(void *arg) {
    int nodes_dispatched = 0;

    //Fill the buffer with the source nodes and signal the other threads if it is full
    for (uint32_t src = 0; src < graph->file_infos->nb_nodes; src++) {
            pthread_mutex_lock(&dispatch_mutex);

            //If the buffer is full, wait for it to be emptied by the other threads
            while ((dispatchhead + 1) % BUFFERSIZE == dispatchtail) {
                pthread_cond_wait(&dispatch_not_full, &dispatch_mutex);
            }
            dispatchbuffer[dispatchhead] = src;
            dispatchhead = (dispatchhead + 1) % BUFFERSIZE;
            pthread_cond_broadcast(&dispatch_not_empty);
            pthread_mutex_unlock(&dispatch_mutex);

            nodes_dispatched++;
    }
    if (verbose){printf("Done dispatching %d nodes\n", nodes_dispatched);}
    pthread_exit(NULL);
}

/*
Function computers
---------------------------

Function executed by the computer threads, it computes the best path to the given node

Input:
----------
None, it takes its source node from the dispatchbuffer

Output:
----------
None, it writes the result in the writebuffer
*/
void *computers(void *arg) {

    // In the unlikely event that all the nodes have been computed, the thread exits
    pthread_mutex_lock(&counter_mutex);
    if (computed_nodes == graph->file_infos->nb_nodes){
        pthread_mutex_unlock(&counter_mutex);
        pthread_exit(NULL);
    } 
    pthread_mutex_unlock(&counter_mutex);

    // Also checks if the graph is null, if it is, exit because cannot compute anything
    if (graph == NULL) {
        printf("Error: graph is null\n");
        error = true;
        pthread_exit(NULL);
    }

    // Allocates the data structure that will be written in the writebuffer
    thread_data_t *data = (thread_data_t *)malloc(sizeof(thread_data_t));
    if (data == NULL) {
        printf("Error: Failed to allocate thread data.\n");
        free(data);
        error = true;
        pthread_exit(NULL);
    }

    // While the read thread is not done, it reads the source node from the dispatchbuffer and does its computing on it
    while (true){

        //Waits for the dispatchbuffer to be accessible and reads the source node 
        pthread_mutex_lock(&dispatch_mutex);
        while (dispatchhead == dispatchtail){
            pthread_cond_wait(&dispatch_not_empty, &dispatch_mutex);
        }
        uint32_t source = dispatchbuffer[dispatchtail];
        dispatchtail = (dispatchtail + 1) % BUFFERSIZE;
        pthread_cond_signal(&dispatch_not_full);
        pthread_mutex_unlock(&dispatch_mutex);

        pthread_mutex_lock(&counter_mutex);
        computed_nodes++;
        pthread_mutex_unlock(&counter_mutex);

        // Computes the best path to the source node and stores it in the data structure
        ford_t *ford = (ford_t *)bellman_ford(graph->file_infos->nb_nodes, graph->file_infos->nb_edges, graph->graph_data, source, verbose);
        if (ford == NULL) {
            printf("Error: Failed to compute best path to node %d.\n", source);
            free(data);
            error = true;
            pthread_exit(NULL);
        }
        mcost_t * max = (mcost_t *)get_max(graph->file_infos->nb_nodes, ford->dist, source);
        if (max == NULL) {
            printf("Error: Failed to get max cost to node %d.\n", source);
            free_ford_struct(ford);
            free(data);
            error = true;
            pthread_exit(NULL);
        }
        int32_t size = graph->file_infos->nb_nodes;
        int32_t * path = (int32_t *)get_path(max->node, source, ford->path, &size);
        if (path == NULL) {
            printf("Error: Failed to get path to node %d.\n", source);
            free_ford_struct(ford);
            free_max_struct(max);
            free(data);
            error = true;
            pthread_exit(NULL);
        }
        data->source = source; data->max = max; data->size = size; data->path = path;data->ford = ford;

        // Writes the data structure in the writebuffer, waits for the buffer if needed 
        pthread_mutex_lock(&write_mutex);
        while ((writehead + 1) % BUFFERSIZE == writetail) {
            pthread_cond_wait(&write_not_full, &write_mutex);
        }
        writebuffer[writehead] = *data;
        writehead = (writehead + 1) % BUFFERSIZE;
        pthread_cond_broadcast(&write_not_empty);
        pthread_mutex_unlock(&write_mutex);

        if (source >= graph->file_infos->nb_nodes - NTHREADS){
            free(data);
            pthread_exit(NULL);
        }   
    }
    pthread_exit(NULL);
}

/*
Function writethread
---------------------------

Function executed by the write thread, it writes the data structure in the writebuffer to the file

Input:
----------
None, it takes its data from the writebuffer

Output:
----------
None, it writes the data to the file
*/
void *writethread(void *arg){

    // Initialization of the variables used in the function and getting the file from the arguments
    FILE *file = (FILE*)arg; thread_data_t *data; bool first_pass = true; 
    
    // While the compute threads are not done, it writes the data structure in the writebuffer to the file
    while (true) {

        // Waits for the writebuffer to be accessible and reads the data structure
        pthread_mutex_lock(&write_mutex);
        while (writehead == writetail) {
            pthread_cond_wait(&write_not_empty, &write_mutex);
            if (written_nodes == graph->file_infos->nb_nodes) {
                pthread_mutex_unlock(&write_mutex);
                pthread_exit(NULL);
            }
        }
        data = &writebuffer[writetail];
        writetail = (writetail + 1) % BUFFERSIZE;
        pthread_cond_signal(&write_not_full);
        pthread_mutex_unlock(&write_mutex);

        // Checks if the data is null, if it is, exits because cannot write anything/ there was a bug
        if (data == NULL) {
            printf("Error: data is null\n");
            error = true;
            pthread_exit(NULL);
        }
        if (show){// Shows the graph computed data in the terminal
            printf("Source node : %d\nDistances : [ ", data->source);
            for (int i = 0; i < graph->file_infos->nb_nodes; i++) {
                printf("%d ", data->ford->dist[i]);
            }
            printf("]\n    Destination : %u\n    Cost : "MY_FORMAT_SPECIFIER"\n    Number of nodes : %d\n    Path: [", data->max->node, data->max->cost, data->size);
            for (int i = 0; i < data->size; i++) {
                printf(" %d", data->path[i]);
            }
            printf(" ]\n");
        } else{
            if (first_pass) {
                uint32_t nb_nodes_be = htonl(graph->file_infos->nb_nodes);
                if (fwrite(&nb_nodes_be, sizeof(uint32_t), 1, file) != 1) {
                    printf("Error: Failed to write source node.\n");
                    error = true;
                    pthread_exit(NULL);
                }
                first_pass = false;
            }
            if (write_to_file(file, data->source, data->max, data->size, data->path) == 1){
                printf("Error writing to file\n");
                error = true;
                pthread_exit(NULL);
            }
        }
        
        free_ford_struct(data->ford);
        free_max_struct(data->max);
        free_path(data->path);

        // Check if all the data has been written, if it has, exits
        pthread_mutex_lock(&written_nodes_mutex);
        written_nodes++;
        if (written_nodes == graph->file_infos->nb_nodes) {
            pthread_mutex_unlock(&written_nodes_mutex);
            pthread_exit(NULL);
        }
        pthread_mutex_unlock(&written_nodes_mutex);
    }
    pthread_exit(NULL);
}
 

/*
Function submain
---------------------------

Function executed by the main thread, it executes the other threads and manages the buffers

Input:
----------
sub_data_t *sub_data : the data structure containing the arguments of the program

Output:
----------
int : 0 if everything went well, 1 if there was an error
*/
int submain(sub_data_t *sub_data) {
    
    // Initialization of the information needed for the function
    // It is passed by the main function in the sub_data structure
    error = false;
    verbose = sub_data->verbose;
    NTHREADS = sub_data->nb_threads;
    BUFFERSIZE = NTHREADS+1;
    if (sub_data->print){show = true;}


    dispatchbuffer = malloc(sizeof(uint32_t) * BUFFERSIZE);
    writebuffer = malloc(sizeof(thread_data_t) * BUFFERSIZE);

    // Creation of the graph structure from the file
    graph = get_file_info(sub_data->inputfile);
    if (graph == NULL) {printf("Error: Failed to get file info.\n");return 1;}
    if (verbose){printf("Number of nodes : %d\nNumber of edges : %d\n", graph->file_infos->nb_nodes, graph->file_infos->nb_edges);}

    // If the number of threads is bigger than the number of nodes, we set the number of threads to
    // the number of nodes, more than that is useless since they will be initialized and killed right away
    if (NTHREADS > graph->file_infos->nb_nodes){
        NTHREADS = graph->file_infos->nb_nodes;
    }

    if (verbose){printf("NTHREADS USED : %d\n", NTHREADS);}
    if (verbose){printf("BUFFERSIZE USED: %d\n", BUFFERSIZE);}
    
    // Initializes the threads, mutexes and conditions
    pthread_t dispatcher, writer;
    pthread_t thread[NTHREADS];

    pthread_mutex_init(&dispatch_mutex, NULL);
    pthread_mutex_init(&write_mutex, NULL); 
    pthread_mutex_init(&written_nodes_mutex, NULL);
    pthread_mutex_init(&counter_mutex, NULL);
    
    pthread_cond_init(&dispatch_not_empty, NULL);
    pthread_cond_init(&dispatch_not_full, NULL);
    pthread_cond_init(&write_not_empty, NULL);
    pthread_cond_init(&write_not_full, NULL);

    // Start dispatch thread
    pthread_create(&dispatcher, NULL, readthread, NULL);

    // Start computer threads
    for (int thrd = 0; thrd < NTHREADS; thrd++) {
        pthread_create(&thread[thrd], NULL, computers, NULL);
    }
    
    // Start write thread
    pthread_create(&writer, NULL, writethread, (void *) sub_data->outputfile);
    if (verbose){printf("All threads created waiting for compute\n");}

    // Wait for computer threads to finish first to avoid a deadlock
    for (int thrd = 0; thrd < NTHREADS; thrd++) {
        pthread_join(thread[thrd], NULL);
        if (verbose){printf("Done thread %d joined\n", thrd);}
    }
    if (verbose){printf("All compute threads joined, waiting for dispatcher and writer\n");}
    
    // Wait for dispatcher thread to finish
    pthread_join(dispatcher, NULL);
    if (verbose){printf("Dispatcher thread joined, waiting for writer\n");}

    // Wait for write thread to finish
    pthread_join(writer, NULL);
    if (verbose){printf("Done last thread joined, %d nodes were fully processed\n", written_nodes);}

    // Cleanup 
    pthread_mutex_destroy(&written_nodes_mutex);
    pthread_mutex_destroy(&dispatch_mutex);
    pthread_mutex_destroy(&counter_mutex);
    pthread_mutex_destroy(&write_mutex);
    
    pthread_cond_destroy(&dispatch_not_empty);
    pthread_cond_destroy(&dispatch_not_full);
    pthread_cond_destroy(&write_not_empty);
    pthread_cond_destroy(&write_not_full);
    
    free_graph_struct(graph);
    free(dispatchbuffer);
    free(writebuffer);
    
    if (error) {printf("An error occured");return 1;}
    return 0;
}

