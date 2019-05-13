#ifndef SIMILARITY_GRAPH_H
#define SIMILARITY_GRAPH_H

#include <stdlib.h>
#include "similarity.h"
#include "file.h"

struct similarity_graph
{
    similarity *head;
    similarity *tail;
    size_t size;
};
typedef struct similarity_graph similarity_graph;

similarity_graph *similarity_graph_create();
void similarity_graph_destroy(similarity_graph *graph);
void similarity_graph_push_front(similarity_graph *graph, similarity *sim);
void similarity_graph_add(similarity_graph *graph, similarity *sim);
similarity *find_largest_similarity(file *first, file *second, int start,
                                    int end, int search_start, int search_end);
similarity_graph *generate_graph(file *first, file *second);
void similarity_graph_print(file *first, file *second,
                            similarity_graph *graph);
void similarity_graph_print_alt(file *first, file *second,
                                similarity_graph *graph);

#endif