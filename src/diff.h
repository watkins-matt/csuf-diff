#ifndef DIFF_H
#define DIFF_H

enum COMMAND_FLAGS
{
    NONE = 0,
    VERSION = 1,     // -v --version
    QUIET_MODE = 2,  // -q --brief
    REPORT_SAME = 4, // -s Reports if files are the same
    SIDE_BY_SIDE = 8,
    LEFT_COLUMN_ONLY = 16,
    SUPPRESS_COMMON_LINES = 32,
    COMBINED_MODE = 64,
    UNIFIED_MODE = 128
};
typedef enum COMMAND_FLAGS COMMAND_FLAGS;

struct command_line_options
{
    char *first_file;
    char *second_file;
    COMMAND_FLAGS flags;
};
typedef struct command_line_options command_line_options;

void print_seperator();
command_line_options *command_line_options_create(const char *first_file, const char *second_file);
void command_line_show_usage_message();
command_line_options *process_command_line_arguments(int argc, char *argv[]);
void command_line_show_version();

#endif