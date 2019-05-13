#ifndef DISCREPANCY_H
#define DISCREPANCY_H

#include "file.h"

enum DISCREPANCY_TYPE
{
    Addition,
    Deletion
};
typedef enum DISCREPANCY_TYPE DISCREPANCY_TYPE;

struct discrepancy
{
    int line_number;
    int total_lines;
    DISCREPANCY_TYPE type;

    struct discrepancy *next;
};
typedef struct discrepancy discrepancy;

discrepancy *discrepancy_create(int line_number, int total_lines, DISCREPANCY_TYPE type);
int discrepancy_last_line(discrepancy *disc);
void file_print_discrepancy(file *first, file *second, discrepancy *disc);

#endif