#ifndef SIMILARITY_H
#define SIMILARITY_H

#include "file.h"

struct similarity
{
    int line_number;
    int total_lines_matched;
    int source_line_number;
    struct similarity *next;
};
typedef struct similarity similarity;

similarity *similarity_create_default();
similarity *similarity_create(int line_number, int total_lines_matched, int source_line_number);
void similarity_destroy(similarity *this);
int similarity_last_line(similarity *sim);
int similarity_print(file *first, file *second, similarity *sim);
void file_print_similarity(file *file, similarity *sim);

#endif