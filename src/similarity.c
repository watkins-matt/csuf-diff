#include "similarity.h"
#include <stdlib.h>
#include <stdio.h>

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

void file_print_similarity(file *file, similarity *sim)
{
    for (int i = sim->line_number; i < sim->line_number + sim->total_lines_matched; i++)
    {
        printf("%02d\t%s", i + 1, file->lines[i]);
    }
}