#ifndef LEPL1503__TEST__
#define LEPL1503__TEST__
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include "../include/graph.h"

typedef struct test_graph{
    uint32_t nb_nodes;
    uint32_t *sources;
    uint32_t *destinations;
    int64_t *costs;
    int32_t *path_sizes;
    int32_t **paths;
} test_graph_t;

//Here are contained all the functions used in the cu_tests.c file, for more information on how they work, read the docstring in said file

void test_default(void);

void test_negative_cost(void);

void test_empty(void);

void test_corrupted(void);

void test_only_zeros(void);

uint32_t btole32(unsigned char *buffer);

int64_t btole64(unsigned char *buffer);

int compare_graphs(test_graph_t *graph1, test_graph_t *graph2);

void init_test_graph_struct(test_graph_t *graph, uint32_t nb_nodes);

void free_test_graph(test_graph_t *graph);

int run_file(FILE *file, test_graph_t *graph);

int check_binary_files_equal(const char *file1, const char *file2);

void test_check_binary_files_equal(void);

void test_get_file_info(void);

void test_bellman_ford(void);

void test_get_max(void);

void test_get_path(void);

int main(int argc, char *argv[]);

#endif
