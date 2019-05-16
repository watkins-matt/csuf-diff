#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "diff.h"
#include "file.h"
#include "similarity.h"
#include "similarity_graph.h"
#include "discrepancy_graph.h"

#define strdup _strdup
#define strcmpi _strcmpi

void print_seperator()
{
    printf("===================================\n");
}

command_line_options *command_line_options_create(const char *first_file, const char *second_file)
{
    command_line_options *options = malloc(sizeof(command_line_options));
    options->first_file = strdup(first_file);
    options->second_file = strdup(second_file);
    options->flags = 0;
    return options;
}

command_line_options *command_line_options_create_default()
{
    command_line_options *options = malloc(sizeof(command_line_options));
    options->first_file = NULL;
    options->second_file = NULL;
    options->flags = 0;
    return options;
}

void command_line_options_destroy(command_line_options *options)
{
    free(options->first_file);
    free(options->second_file);
    free(options);
}

void command_line_show_usage_message()
{
    printf("[Usage] diff.exe [Options] [First File] [Second File]\n");
    exit(1);
}

command_line_options *process_command_line_arguments(int argc, char *argv[])
{
    if (argc < 3)
    {
        command_line_show_usage_message();
    }

    // This will not crash because we are guaranteed to have >= 3 arguments.
    // The last two arguments will always be the files to process.
    command_line_options *options = command_line_options_create_default();
    options->first_file = strdup(argv[argc - 2]);
    options->second_file = strdup(argv[argc - 1]);

    if (options->first_file[0] == '-' || options->second_file[0] == '-')
    {
        printf("Invalid files specified, must specify two files as the final two parameters.\n");
        command_line_show_usage_message();
    }

    for (int i = 1; i < argc - 2; i++)
    {
        const char *argument = argv[i];

        if (strcmpi(argument, "-v") == 0 || strcmpi(argument, "--version") == 0)
        {
            options->flags |= VERSION;
        }

        else if (strcmpi(argument, "-q") == 0 || strcmpi(argument, "--brief") == 0)
        {
            options->flags |= QUIET_MODE;
        }

        else if (strcmpi(argument, "-s") == 0)
        {
            options->flags |= REPORT_SAME;
        }

        else if (strcmpi(argument, "-y") == 0 || strcmpi(argument, "--side-by-side") == 0)
        {
            options->flags |= SIDE_BY_SIDE;
        }

        else if (strcmpi(argument, "--left-column") == 0)
        {
            options->flags |= LEFT_COLUMN_ONLY;
        }

        else if (strcmpi(argument, "--suppress-common-lines") == 0)
        {
            options->flags |= SUPPRESS_COMMON_LINES;
        }

        else
        {
            printf("Invalid argument: %s\n", argument);
            command_line_show_usage_message();
        }
    }

    return options;
}

void print_diff(file *first, file *second, COMMAND_FLAGS flags, similarity_graph *sim_graph, discrepancy_graph *disc_graph)
{
    if (flags & QUIET_MODE || flags & REPORT_SAME)
    {
        if (flags & REPORT_SAME && disc_graph->size == 0)
        {
            printf("Files %s and %s are the same.", first->file_name, second->file_name);
        }

        else if (disc_graph->size > 0)
        {
            printf("Files %s and %s are different.", first->file_name, second->file_name);
        }
        return;
    }

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
                if (!(flags & SUPPRESS_COMMON_LINES))
                    file_print_similarity(second, sim, flags);
                line_number += sim->total_lines_matched;
                sim = sim->next;
            }

            else
            {
                file_print_discrepancy(first, second, disc, line_number, flags);
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
            if (!(flags & SUPPRESS_COMMON_LINES))
                file_print_similarity(second, sim, flags);
            line_number += sim->total_lines_matched;
            sim = sim->next;
        }

        // Only discrepancies left
        else if (sim == NULL)
        {
            file_print_discrepancy(first, second, disc, line_number, flags);
            line_number += disc->total_lines;
            disc = disc->next;
        }
    }
}

void command_line_show_version()
{
    printf("diff.exe 1.0\n(C)2019 Matt Watkins\nLicensed under the MIT License.");
    exit(1);
}

int main(int argc, char *argv[])
{
    command_line_options *options = process_command_line_arguments(argc, argv);

    if (options->flags & VERSION)
    {
        command_line_show_version();
    }

    if (options->flags & COMBINED_MODE && options->flags & UNIFIED_MODE)
    {
        printf("Cannot use both combined and unifed mode at the same time.\n");
        command_line_show_usage_message();
    }

    file *first = file_open(options->first_file);
    file *second = file_open(options->second_file);

    similarity_graph *graph = generate_graph(first, second);
    discrepancy_graph *discrepancy_graph = generate_discrepancy_graph(first, second, graph);

    print_diff(first, second, options->flags, graph, discrepancy_graph);
    
    // Clean everything up
    command_line_options_destroy(options);
    discrepancy_graph_destroy(discrepancy_graph);
    similarity_graph_destroy(graph);
    file_close(first);
    file_close(second);
    return 0;
}