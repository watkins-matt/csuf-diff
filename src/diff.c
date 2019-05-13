#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file.h"
#include "similarity.h"
#include "similarity_graph.h"
#include "discrepancy_graph.h"

#define strdup _strdup

void print_seperator()
{
    printf("===================================\n");
}

struct command_line_options
{
    char *first_file;
    char *second_file;
};
typedef struct command_line_options command_line_options;

command_line_options *command_line_options_create(const char *first_file, const char *second_file)
{
    command_line_options *options = malloc(sizeof(command_line_options));
    options->first_file = strdup(first_file);
    options->second_file = strdup(second_file);
    return options;
}

command_line_options *command_line_options_create_default()
{
    command_line_options *options = malloc(sizeof(command_line_options));
    options->first_file = NULL;
    options->second_file = NULL;
    return options;
}

void command_line_options_destroy(command_line_options *options)
{
    free(options->first_file);
    free(options->second_file);
    free(options);
}

command_line_options *process_command_line_arguments(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("[Usage] diff.exe [options] [file_a] [file_b]\n");
        exit(1);
    }

    // This will not crash because we are guaranteed to have >= 3 arguments.
    // The last two arguments will always be the files to process.
    command_line_options *options = command_line_options_create_default();
    options->first_file = strdup(argv[argc - 2]);
    options->second_file = strdup(argv[argc - 1]);

    //     for (int i = argc-1; i > 1; i--)
    //     {
    //         options->second_file = strdup
    //     }
    return options;
}

void print_diff(file *first, file *second, similarity_graph *sim_graph, discrepancy_graph *disc_graph)
{
    int line_number = 0;
    int total_lines = second->line_count;

    similarity *sim = sim_graph->head;
    discrepancy *disc = disc_graph->head;

    while (line_number < total_lines)
    {
        if (sim != NULL && disc != NULL)
        {
            int sim_first = sim->line_number <= disc->line_number;
            if (sim_first)
            {
                file_print_similarity(second, sim);
                line_number += sim->total_lines_matched;
                sim = sim->next;
            }

            else
            {
                file_print_discrepancy(first, second, disc);
                line_number += disc->total_lines;
                disc = disc->next;
            }

            // // We may need to backtrack if a similarity was longer than a deletion
            if (disc != NULL && disc->line_number < line_number)
            {
                line_number = disc->line_number;
            }

            if (sim != NULL && sim->line_number < line_number)
            {
                line_number = sim->line_number;
            }
        }

        // Only similarities left
        else if (disc == NULL)
        {
            file_print_similarity(second, sim);
            line_number += sim->total_lines_matched;
            sim = sim->next;
        }

        // Only discrepancies left
        else if (sim == NULL)
        {
            file_print_discrepancy(first, second, disc);
            line_number += disc->total_lines;
            disc = disc->next;
        }
    }
}

int main(int argc, char *argv[])
{
    command_line_options *options = process_command_line_arguments(argc, argv);

    file *first = file_open(options->first_file);
    file *second = file_open(options->second_file);

    command_line_options_destroy(options);

    similarity_graph *graph = generate_graph(first, second);
    similarity_graph_print(first, second, graph);
    print_seperator();

    discrepancy_graph *discrepancy_graph = generate_discrepancy_graph(first, second, graph);
    discrepancy_graph_print(first, second, discrepancy_graph);

    print_seperator();
    print_diff(first, second, graph, discrepancy_graph);

    discrepancy_graph_destroy(discrepancy_graph);
    similarity_graph_destroy(graph);

    file_close(first);
    file_close(second);
    return 0;
}