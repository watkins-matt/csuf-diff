#include "file.h"

#define _CRT_SECURE_NO_WARNINGS
#define strdup _strdup

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void file_print(file *this)
{
    for (int i = 0; i < this->line_count; i++)
    {
        printf("%s", this->lines[i]);
    }
}

file *file_open(const char *file_name)
{
    const int max_line_size =
        256; // TODO: Allow for variable line sizes using realloc
    char *line_buffer = malloc(max_line_size);

    file *this = malloc(sizeof(file));
    this->line_count = 0;
    this->file_name = strdup(file_name);

    FILE *file_to_read = fopen(file_name, "r");
    while (fgets(line_buffer, max_line_size, file_to_read) != NULL)
    {
        this->lines[this->line_count] = strdup(line_buffer);
        this->line_count++;

        if (this->line_count >= MAX_LINES)
        {
            printf("Error, file %s has greater than the maximum number of lines, total lines will be capped.\n", file_name);
            break;
        }
    }

    fclose(file_to_read);
    return this;
}

void file_close(file *this)
{
    free(this->file_name);
    for (int i = 0; i < this->line_count; i++)
    {
        free(this->lines[i]);
    }
    free(this);
}

int file_find_line(file *this, const char *line, int start_limit, int end_limit)
{
    assert(end_limit <= this->line_count);
    for (int i = start_limit; i < end_limit; i++)
    {
        if (strcmp(this->lines[i], line) == 0)
        {
            return i;
        }
    }

    return -1;
}

int get_matched_line_count(file *first, file *second, int first_line_index, int second_line_index)
{
    int match_count = 0;

    while (first_line_index < first->line_count && second_line_index < second->line_count &&
           strcmp(first->lines[first_line_index], second->lines[second_line_index]) == 0)
    {
        first_line_index++;
        second_line_index++;
        match_count++;
    }

    return match_count;
}

int find_longest_match(file *first, file *second, const int line_number, const int search_start, const int search_end)
{
    int max_lines_matched = 0;
    int best_match_line = -1;
    int line_match = file_find_line(second, first->lines[line_number], search_start, search_end);

    while (line_match != -1)
    {
        int matched_line_count =
            get_matched_line_count(first, second, line_number, line_match);

        if (matched_line_count > max_lines_matched)
        {
            max_lines_matched = matched_line_count;
            best_match_line = line_match;
        }

        line_match = file_find_line(
            second, first->lines[line_number], line_match + matched_line_count, search_end);
    }

    return best_match_line;
}