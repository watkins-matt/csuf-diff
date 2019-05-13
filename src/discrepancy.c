#include "discrepancy.h"
#include <stdlib.h>
#include <stdio.h>

discrepancy *discrepancy_create(int line_number, int total_lines, DISCREPANCY_TYPE type)
{
    discrepancy *sim = malloc(sizeof(discrepancy));
    sim->line_number = line_number;
    sim->total_lines = total_lines;
    sim->type = type;
    sim->next = NULL;
    return sim;
}

int discrepancy_last_line(discrepancy *disc)
{
    return disc->line_number + disc->total_lines;
}

void file_print_discrepancy(file *first, file *second, discrepancy *disc)
{
    for (int i = disc->line_number; i < disc->line_number + disc->total_lines; i++)
    {
        if (disc->type == Addition)
        {
            printf("%02d\t>%s", i + 1, second->lines[i]);
        }

        else
        {
            printf("%02d\t<%s", i + 1, first->lines[i]);
        }
    }
}
