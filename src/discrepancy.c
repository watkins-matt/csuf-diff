#include "discrepancy.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

void file_print_discrepancy(file *first, file *second, discrepancy *disc, int line_number, COMMAND_FLAGS flags)
{
    if (!(flags & SIDE_BY_SIDE))
    {
        switch (disc->type)
        {
        case Addition:
            printf("%da%d,%d\n", line_number, disc->line_number, disc->line_number + disc->total_lines);
            break;
        case Deletion:
            printf("%d,%dd%d\n", disc->line_number, disc->line_number + disc->total_lines, line_number);
            break;
        }
    }

    for (int i = disc->line_number; i < disc->line_number + disc->total_lines; i++)
    {
        if (disc->type == Addition)
        {
            char *second_line = strdup(second->lines[i]);
            if (second_line[strlen(second_line) - 1] == '\n')
            {
                second_line[strlen(second_line) - 1] = '\0';
            }

            if (flags & SIDE_BY_SIDE)
            {
                printf("%38.38s >%38.38s\n", "", second_line);
            }

            else
            {
                printf(">%s\n", second_line);
            }
            free(second_line);
        }

        else // Deletion
        {
            char *first_line = strdup(first->lines[i]);
            if (first_line[strlen(first_line) - 1] == '\n')
            {
                first_line[strlen(first_line) - 1] = '\0';
            }

            if (flags & SIDE_BY_SIDE)
            {
                printf("<%38.38s %38.38s\n", first_line, "");
            }

            else
            {
                printf("<%s\n", first_line);
            }
            free(first_line);
        }
    }
}
