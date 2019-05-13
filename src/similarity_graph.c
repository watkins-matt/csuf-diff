#include "similarity_graph.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

similarity_graph *similarity_graph_create()
{
    similarity_graph *graph = malloc(sizeof(struct similarity_graph));
    graph->head = NULL;
    graph->tail = NULL;
    graph->size = 0;
    return graph;
}

void similarity_graph_destroy(similarity_graph *graph)
{
    // TODO: Fix freeing the nodes
    // for (similarity *it = graph->head; it != NULL; it = it->next)
    // {
    //     free(it);
    // }
    free(graph);
}

void similarity_graph_push_front(similarity_graph *graph, similarity *sim)
{
    // This is the first node being added to the graph
    if (graph->head == NULL)
    {
        assert(graph->tail == NULL);
        graph->head = sim;
        graph->tail = sim;
        sim->next = NULL; // There are no other nodes
    }

    else
    {
        assert(graph->tail != NULL);
        sim->next = graph->head;
        graph->head = sim;
    }

    graph->size++;
}

void similarity_graph_add(similarity_graph *graph, similarity *sim)
{
    // This is the first node being added to the graph
    if (graph->head == NULL)
    {
        assert(graph->tail == NULL);
        graph->head = sim;
        graph->tail = sim;
        sim->next = NULL; // There are no other nodes
    }

    else
    {
        assert(graph->tail != NULL);

        if (sim->line_number < graph->head->line_number)
        {
            sim->next = graph->head;
            graph->head = sim;
        }

        else if (sim->line_number > graph->tail->line_number)
        {
            graph->tail->next = sim;
            graph->tail = sim;
        }

        else
        {
            similarity *last = graph->head;
            for (similarity *iterator = graph->head->next; iterator != NULL; iterator = iterator->next)
            {
                if (sim->line_number < iterator->line_number)
                {
                    assert(last->next = iterator);

                    // Tried to add an invalid similarity (out of order)
                    // TODO: Better error checking here
                    if (sim->source_line_number > iterator->source_line_number)
                    {
                        similarity_destroy(sim);
                        return;
                    }

                    last->next = sim;
                    sim->next = iterator;
                    break;
                }
                last = iterator;
            }
        }
    }

    graph->size++;
}

similarity *find_largest_similarity(file *first, file *second, int start,
                                    int end, int search_start, int search_end)
{

    similarity *sim = similarity_create_default();

    for (int i = start; i < end; i++)
    {

        int match = find_longest_match(first, second, i, search_start, search_end);
        int matched_lines = get_matched_line_count(first, second, i, match);

        if (matched_lines > sim->total_lines_matched)
        {
            sim->line_number = match;
            sim->total_lines_matched = matched_lines;
            sim->source_line_number = i;
        }
    }

    if (sim->line_number == -1)
    {
        similarity_destroy(sim);
        return NULL;
    }

    return sim;
}

similarity_graph *build_graph(file *first, file *second, similarity_graph *graph, int start, int end, int search_start_limit, int search_end_limit)
{
    similarity *largest_similarity =
        find_largest_similarity(first, second, start, end, search_start_limit, search_end_limit);

    if (largest_similarity != NULL)
    {
        if (graph->head != NULL && largest_similarity->line_number < graph->head->line_number)
        {
            similarity_graph_push_front(graph, largest_similarity);
        }

        else
        {
            similarity_graph_add(graph, largest_similarity);
        }

        int before_start_line = start;
        int before_end_line = largest_similarity->source_line_number;

        // Recursively search above the similarity for greatest size matches
        if (before_start_line < before_end_line)
        {
            graph = build_graph(first, second, graph,
                                before_start_line, before_end_line, search_start_limit, largest_similarity->line_number);
        }

        int after_start_line = largest_similarity->source_line_number + largest_similarity->total_lines_matched;
        int after_end_line = end;

        // Recursively search below the similarity for greatest size matches
        if (after_start_line < after_end_line)
        {
            graph = build_graph(first, second, graph,
                                after_start_line, after_end_line, largest_similarity->line_number + largest_similarity->total_lines_matched, search_end_limit);
        }
    }

    return graph;
}

similarity_graph *generate_graph(file *first, file *second)
{
    similarity_graph *graph = similarity_graph_create();
    graph = build_graph(first, second, graph, 0,
                        first->line_count, 0, second->line_count);
    return graph;
}

void similarity_graph_print(file *first, file *second,
                            similarity_graph *graph)
{
    int i = 0;

    for (similarity *sim = graph->head; sim != NULL; sim = sim->next, i++)
    {
        printf("Similarity %d: File 1, Line %d matches %d lines in File 2 at line %d\n", i, sim->source_line_number, sim->total_lines_matched, sim->line_number);
    }
}

void similarity_graph_print_alt(file *first, file *second,
                                similarity_graph *graph)
{
    int line = 0;
    int line_start = 0;

    for (similarity *sim = graph->head; sim != NULL; sim = sim->next, line++)
    {
        for (int i = line_start; i < sim->line_number; i++)
        {
            printf("%02d\t>%s", i + 1, second->lines[i]);
        }

        for (int i = sim->line_number; i < sim->line_number + sim->total_lines_matched; i++)
        {
            printf("%02d\t%s", i + 1, second->lines[i]);
        }

        line_start = sim->line_number + sim->total_lines_matched;

        if (sim->next == NULL)
        {
            // Print the rest of the additions in the file
            for (int i = sim->line_number + sim->total_lines_matched; i < second->line_count; i++)
            {
                printf("%02d\t%s", i + 1, second->lines[i]);
            }
        }
    }
}