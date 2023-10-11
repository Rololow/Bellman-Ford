#ifndef LEPL1503__GRAPHMT__
#define LEPL1503__GRAPHMT__

#include <pthread.h>


/*
This file contains the functions used for the multithreading and their basic description,
for more information on the functions refer to the docstring in the mtgraph.c file
*/

// Definition of the different structures used for the multithreading
typedef struct thread_data{
    uint32_t source;
    mcost_t * max;
    int32_t size;
    int32_t * path;
    ford_t * ford;
} thread_data_t;

typedef struct sub_data{
    FILE * inputfile;
    FILE * outputfile;
    uint8_t nb_threads;
    bool print;
    bool verbose;
} sub_data_t;

/*
Function : readthread
Description : This function is used by a thread to dispatch nodes to compute threads
*/
void *readthread(void *arg);

/*
Function : computethread
Description : This function is used by most threads to compute on the graph
*/
void *computers(void *arg);

/*
Function : writethread
Description : This function is used by a thread that writes the result in the file
*/
void *writethread(void *arg);

/*
Function : submain
Description : This function is used to be the bridge between the main and the threads
*/
int submain(sub_data_t * data); 
#endif // LEPL1503__GRAPHMT__