#ifndef DISCREPANCY_GRAPH_H
#define DISCREPANCY_GRAPH_H

#include <stdlib.h>
#include "discrepancy.h"
#include "file.h"
#include "similarity_graph.h"

struct discrepancy_graph
{
    discrepancy *head;
    discrepancy *tail;
    size_t size;
};
typedef struct discrepancy_graph discrepancy_graph;

discrepancy_graph *discrepancy_graph_create();
void discrepancy_graph_destroy(discrepancy_graph *graph);
void discrepancy_graph_add(discrepancy_graph *graph, discrepancy *sim);
void discrepancy_graph_print(file *first, file *second,
                             discrepancy_graph *graph);
discrepancy_graph *generate_discrepancy_graph(file *first, file *second, similarity_graph *sim_graph);

#endif