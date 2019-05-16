#include "discrepancy_graph.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

discrepancy_graph *discrepancy_graph_create()
{
    discrepancy_graph *graph = malloc(sizeof(struct discrepancy_graph));
    graph->head = NULL;
    graph->tail = NULL;
    graph->size = 0;
    return graph;
}

void discrepancy_graph_destroy(discrepancy_graph *graph)
{
    discrepancy *it = graph->head;
    while (it != NULL)
    {
        discrepancy *temp = it;
        it = it->next;
        free(temp);
    }

    free(graph);
}

void discrepancy_graph_add(discrepancy_graph *graph, discrepancy *sim)
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

        // Insert the discrepancy in the correct position
        else
        {
            discrepancy *last = graph->head;
            for (discrepancy *iterator = graph->head->next; iterator != NULL; iterator = iterator->next)
            {
                if (sim->line_number <= iterator->line_number)
                {
                    assert(last->next = iterator);
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

void discrepancy_graph_print(file *first, file *second,
                             discrepancy_graph *graph)
{
    int i = 0;

    for (discrepancy *disc = graph->head; disc != NULL; disc = disc->next, i++)
    {
        switch (disc->type)
        {
        case Addition:
            printf("Discrepancy %d: File 2, added lines %d to %d\n", i, disc->line_number, disc->line_number + disc->total_lines);
            break;
        case Deletion:
            printf("Discrepancy %d: File 1, deleted lines %d to %d\n", i, disc->line_number, disc->line_number + disc->total_lines);
            break;
        }
    }
}

discrepancy_graph *generate_discrepancy_graph(file *first, file *second, similarity_graph *sim_graph)
{
    discrepancy_graph *graph = discrepancy_graph_create();
    int line_number = 0;

    similarity *first_sim = sim_graph->head;
    if (first_sim->line_number > 0)
    {
        discrepancy *disc = discrepancy_create(line_number, first_sim->line_number, Addition);
        discrepancy_graph_add(graph, disc);
    }

    // First run for deletions
    for (similarity *it = sim_graph->head; it != NULL; it = it->next)
    {
        int current_line = it->source_line_number;

        if (line_number < current_line)
        {
            discrepancy *disc = discrepancy_create(line_number, current_line - line_number, Deletion);
            disc->source_line_number = current_line;
            discrepancy_graph_add(graph, disc);
        }

        line_number = it->source_line_number + it->total_lines_matched;
    }

    // Second run for additions
    for (similarity *it = sim_graph->head; it != NULL; it = it->next)
    {
        int current_line = it->line_number;

        if (line_number < current_line)
        {
            discrepancy *disc = discrepancy_create(line_number, current_line - line_number, Addition);
            discrepancy_graph_add(graph, disc);
        }

        line_number = it->line_number + it->total_lines_matched;

        // Consider the rest of the file to be an addition if there are greater than zero lines left
        if (it->next == NULL && (second->line_count - line_number) > 0)
        {
            discrepancy *disc = discrepancy_create(line_number, second->line_count - line_number, Addition);
            discrepancy_graph_add(graph, disc);
        }
    }
    return graph;
}
