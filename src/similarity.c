#include "similarity.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

similarity *similarity_create_default()
{
    similarity *sim = malloc(sizeof(similarity));
    sim->line_number = -1;
    sim->total_lines_matched = 0;
    sim->source_line_number = -1;
    sim->next = NULL;
    return sim;
}

similarity *similarity_create(int line_number, int total_lines_matched, int source_line_number)
{
    similarity *sim = malloc(sizeof(similarity));
    sim->line_number = line_number;
    sim->total_lines_matched = total_lines_matched;
    sim->source_line_number = source_line_number;
    sim->next = NULL;
    return sim;
}

void similarity_destroy(similarity *this)
{
    free(this);
}

int similarity_last_line(similarity *sim)
{
    return sim->line_number + sim->total_lines_matched;
}

int similarity_print(file *first, file *second, similarity *sim)
{
    for (int i = sim->line_number; i < sim->total_lines_matched; i++)
    {
        printf("%d. %s", i, first->lines[i]);
    }

    return sim->line_number + sim->total_lines_matched;
}

void file_print_similarity(file *file, similarity *sim, COMMAND_FLAGS flags)
{
    for (int i = sim->line_number; i < sim->line_number + sim->total_lines_matched; i++)
    {
        char *line = strdup(file->lines[i]);
        if (line[strlen(line) - 1] == '\n')
        {
            line[strlen(line) - 1] = '\0';
        }

        if (flags & SIDE_BY_SIDE)
        {
            if (flags & LEFT_COLUMN_ONLY)
            {
               printf("%38.38s\n", line); 
            }

            else
            {
                printf("%38.38s %38.38s\n", line, line); 
            }
        }

        else
        {
            printf("%s\n", line);
        }

        free(line);
    }
}