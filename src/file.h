#ifndef FILE_H
#define FILE_H

#define MAX_LINES 500

struct file
{
    char *file_name;
    char *lines[MAX_LINES];
    int line_count;
};
typedef struct file file;

void file_print(file *this);
file *file_open(const char *file_name);
void file_close(file *this);

int get_matched_line_count(file *first, file *second, int first_line_index, int second_line_index);
int file_find_line(file *this, const char *line, int start_limit, int end_limit);
int find_longest_match(file *first, file *second, const int line_number, const int search_start, const int search_end);

#endif