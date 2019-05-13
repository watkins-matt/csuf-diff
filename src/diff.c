#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINES 500
#define MAX_DIFFS MAX_LINES

struct file
{
    char *file_name;
    char *lines[MAX_LINES];
    int line_count;
};

typedef struct file file;

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
        200; // TODO: Allow for variable line sizes using realloc
    char *line_buffer = malloc(max_line_size);
    file *this = malloc(sizeof(file));

    this->line_count = 0;
    this->file_name = strdup(file_name);
    FILE *file_to_read = fopen(file_name, "r");

    while (fgets(line_buffer, max_line_size, file_to_read) != NULL)
    {
        this->lines[this->line_count] = strdup(line_buffer);
        this->line_count++;
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

struct similarity
{
    int line_number;
    int total_lines_matched;
    int source_line_number;
    struct similarity *next;
};
typedef struct similarity similarity;

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

struct similarity_graph
{
    similarity *head;
    similarity *tail;
    size_t size;
};
typedef struct similarity_graph similarity_graph;

struct coverage_graph
{
    int coverage[MAX_LINES];
    int total_lines;

    int longest_match_index;
    int longest_match_line_count;
};
typedef struct coverage_graph coverage_graph;

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

similarity_graph *similarity_graph_create()
{
    similarity_graph *graph = malloc(sizeof(struct similarity_graph));
    graph->head = NULL;
    graph->tail = NULL;
    graph->size = 0;
    return graph;
}

void similarity_graph_destroy(similarity_graph *graph)
{
    // TODO: Fix freeing the nodes
    // for (similarity *it = graph->head; it != NULL; it = it->next)
    // {
    //     free(it);
    // }
    free(graph);
}

void similarity_graph_push_front(similarity_graph *graph, similarity *sim)
{
    // This is the first node being added to the graph
    if (graph->head == NULL)
    {
        assert(graph->tail == NULL);
        graph->head = sim;
        graph->tail = sim;
        sim->next = NULL; // There are no other nodes
    }

    else
    {
        assert(graph->tail != NULL);
        sim->next = graph->head;
        graph->head = sim;
    }

    graph->size++;
}

void similarity_graph_add(similarity_graph *graph, similarity *sim)
{
    // This is the first node being added to the graph
    if (graph->head == NULL)
    {
        assert(graph->tail == NULL);
        graph->head = sim;
        graph->tail = sim;
        sim->next = NULL; // There are no other nodes
    }

    else
    {
        assert(graph->tail != NULL);
        // graph->tail->next = sim;
        // graph->tail = sim;

        if (sim->line_number < graph->head->line_number)
        {
            sim->next = graph->head;
            graph->head = sim;
        }

        else if (sim->line_number > graph->tail->line_number)
        {
            graph->tail->next = sim;
            graph->tail = sim;
        }

        else
        {
            similarity *last = graph->head;
            for (similarity *iterator = graph->head->next; iterator != NULL; iterator = iterator->next)
            {
                if (sim->line_number < iterator->line_number)
                {
                    assert(last->next = iterator);

                    // Tried to add an invalid similarity (out of order)
                    // TODO: Better error checking here
                    if (sim->source_line_number > iterator->source_line_number)
                    {
                        similarity_destroy(sim);
                        return;
                    }

                    last->next = sim;
                    sim->next = iterator;
                    break;
                }
                last = iterator;
            }
        }
    }

    graph->size++;
}

similarity *find_largest_similarity(file *first, file *second, int start,
                                    int end, int search_start, int search_end)
{

    similarity *sim = similarity_create_default();

    for (int i = start; i < end; i++)
    {
        // // The line can be found in both the first and the second file
        // if (differences->diffs[i] != -1)
        // {
        int match = find_longest_match(first, second, i, search_start, search_end);
        int matched_lines = get_matched_line_count(first, second, i, match);

        if (matched_lines > sim->total_lines_matched)
        {
            sim->line_number = match;
            sim->total_lines_matched = matched_lines;
            sim->source_line_number = i;
        }

        // i corresponds to the line in the first file, the match index is
        // diffs[i]; we're trying to transform
        // similarity *sim = similarity_create(longest_match,
        //                                     number_of_matching_lines);
        // similarity_graph_add(graph, sim);

        // The number of matching lines are accounted for so we can skip
        // ahead
        // i += number_of_matching_lines - 1;
        // }
    }

    if (sim->line_number == -1)
    {
        similarity_destroy(sim);
        return NULL;
    }

    return sim;
}

similarity_graph *build_graph(file *first, file *second, similarity_graph *graph, int start, int end, int search_start_limit, int search_end_limit)
{
    similarity *largest_similarity =
        find_largest_similarity(first, second, start, end, search_start_limit, search_end_limit);

    if (largest_similarity != NULL)
    {
        if (graph->head != NULL && largest_similarity->line_number < graph->head->line_number)
        {
            similarity_graph_push_front(graph, largest_similarity);
        }

        else
        {
            similarity_graph_add(graph, largest_similarity);
        }

        int before_start_line = start;
        int before_end_line = largest_similarity->source_line_number;

        // Recursively search above the similarity for greatest size matches
        if (before_start_line < before_end_line)
        {
            graph = build_graph(first, second, graph,
                                before_start_line, before_end_line, search_start_limit, largest_similarity->line_number);
        }

        int after_start_line = largest_similarity->source_line_number + largest_similarity->total_lines_matched;
        int after_end_line = end;

        // Recursively search below the similarity for greatest size matches
        if (after_start_line < after_end_line)
        {
            graph = build_graph(first, second, graph,
                                after_start_line, after_end_line, largest_similarity->line_number + largest_similarity->total_lines_matched, search_end_limit);
        }
    }

    return graph;
}

similarity_graph *generate_graph(file *first, file *second)
{
    similarity_graph *graph = similarity_graph_create();
    graph = build_graph(first, second, graph, 0,
                        first->line_count, 0, second->line_count);
    return graph;
}

int similarity_print(file *first, file *second, similarity *sim)
{
    for (int i = sim->line_number; i < sim->total_lines_matched; i++)
    {
        printf("%d. %s", i, first->lines[i]);
    }

    return sim->line_number + sim->total_lines_matched;
}

void similarity_graph_print(file *first, file *second,
                            similarity_graph *graph)
{
    int i = 0;

    for (similarity *sim = graph->head; sim != NULL; sim = sim->next, i++)
    {
        printf("Similarity %d: File 1, Line %d matches %d lines in File 2 at line %d\n", i, sim->source_line_number, sim->total_lines_matched, sim->line_number);
    }
}

void similarity_graph_print_alt(file *first, file *second,
                                similarity_graph *graph)
{
    int line = 0;
    int line_start = 0;

    for (similarity *sim = graph->head; sim != NULL; sim = sim->next, line++)
    {
        for (int i = line_start; i < sim->line_number; i++)
        {
            printf("%02d\t>%s", i + 1, second->lines[i]);
        }

        for (int i = sim->line_number; i < sim->line_number + sim->total_lines_matched; i++)
        {
            printf("%02d\t%s", i + 1, second->lines[i]);
        }

        line_start = sim->line_number + sim->total_lines_matched;

        if (sim->next == NULL)
        {
            // Print the rest of the additions in the file
            for (int i = sim->line_number + sim->total_lines_matched; i < second->line_count; i++)
            {
                printf("%02d\t%s", i + 1, second->lines[i]);
            }
        }
    }
}

void print_seperator()
{
    printf("===================================\n");
}

enum DISCREPANCY_TYPE
{
    Addition,
    Deletion
};
typedef enum DISCREPANCY_TYPE DISCREPANCY_TYPE;

// Build a discrepancy graph
struct discrepancy
{
    int line_number;
    int total_lines;
    DISCREPANCY_TYPE type;

    struct discrepancy *next;
};
typedef struct discrepancy discrepancy;

discrepancy *discrepancy_create(int line_number, int total_lines, DISCREPANCY_TYPE type)
{
    discrepancy *sim = malloc(sizeof(discrepancy));
    sim->line_number = line_number;
    sim->total_lines = total_lines;
    sim->type = type;
    sim->next = NULL;
    return sim;
}

struct discrepancy_graph
{
    discrepancy *head;
    discrepancy *tail;
    size_t size;
};
typedef struct discrepancy_graph discrepancy_graph;

discrepancy_graph *discrepancy_graph_create()
{
    discrepancy_graph *graph = malloc(sizeof(struct discrepancy_graph));
    graph->head = NULL;
    graph->tail = NULL;
    graph->size = 0;
    return graph;
}

void discrepancy_graph_destroy(discrepancy_graph *graph)
{
    // BUGBUG: Fix freeing the nodes
    // for (discrepancy *it = graph->head; it != NULL; it = it->next)
    // {

    //     free(it);
    // }
    free(graph);
}

void discrepancy_graph_add(discrepancy_graph *graph, discrepancy *sim)
{
    // This is the first node being added to the graph
    if (graph->head == NULL)
    {
        assert(graph->tail == NULL);
        graph->head = sim;
        graph->tail = sim;
        sim->next = NULL; // There are no other nodes
    }

    else
    {
        assert(graph->tail != NULL);

        if (sim->line_number < graph->head->line_number)
        {
            sim->next = graph->head;
            graph->head = sim;
        }

        else if (sim->line_number > graph->tail->line_number)
        {
            graph->tail->next = sim;
            graph->tail = sim;
        }

        // Insert the discrepancy in the correct position
        else
        {
            discrepancy *last = graph->head;
            for (discrepancy *iterator = graph->head->next; iterator != NULL; iterator = iterator->next)
            {
                if (sim->line_number <= iterator->line_number)
                {
                    assert(last->next = iterator);
                    last->next = sim;
                    sim->next = iterator;
                    break;
                }
                last = iterator;
            }
        }
    }

    graph->size++;
}

void discrepancy_graph_print(file *first, file *second,
                             discrepancy_graph *graph)
{
    int i = 0;

    for (discrepancy *disc = graph->head; disc != NULL; disc = disc->next, i++)
    {
        switch (disc->type)
        {
        case Addition:
            printf("Discrepancy %d: File 2, added lines %d to %d\n", i, disc->line_number, disc->line_number + disc->total_lines);
            break;
        case Deletion:
            printf("Discrepancy %d: File 1, deleted lines %d to %d\n", i, disc->line_number, disc->line_number + disc->total_lines);
            break;
        }
    }
}

void file_print_similarity(file *file, similarity *sim)
{
    for (int i = sim->line_number; i < sim->line_number + sim->total_lines_matched; i++)
    {
        printf("%02d\t%s", i + 1, file->lines[i]);
    }
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

discrepancy_graph *generate_discrepancy_graph(file *first, file *second, similarity_graph *sim_graph)
{
    discrepancy_graph *graph = discrepancy_graph_create();
    int line_number = 0;

    similarity *first_sim = sim_graph->head;
    if (first_sim->line_number > 0)
    {
        discrepancy *disc = discrepancy_create(line_number, first_sim->line_number, Addition);
        discrepancy_graph_add(graph, disc);
    }

    // First run for deletions
    for (similarity *it = sim_graph->head; it != NULL; it = it->next)
    {
        int current_line = it->source_line_number;

        if (line_number < current_line)
        {
            discrepancy *disc = discrepancy_create(line_number, current_line - line_number, Deletion);
            discrepancy_graph_add(graph, disc);
        }

        line_number = it->source_line_number + it->total_lines_matched;
    }

    // Second run for additions
    for (similarity *it = sim_graph->head; it != NULL; it = it->next)
    {
        int current_line = it->line_number;

        if (line_number < current_line)
        {
            discrepancy *disc = discrepancy_create(line_number, current_line - line_number, Addition);
            discrepancy_graph_add(graph, disc);
        }

        line_number = it->line_number + it->total_lines_matched;

        // Consider the rest of the file to be an addition
        if (it->next == NULL)
        {
            discrepancy *disc = discrepancy_create(line_number, second->line_count - line_number, Addition);
            discrepancy_graph_add(graph, disc);
        }
    }
    return graph;
}

struct command_line_options
{
    char *first_file;
    char *second_file;
};
typedef struct command_line_options command_line_options;

command_line_options *command_line_options_create(const char *first_file, const char *second_file)
{
    command_line_options *options = malloc(sizeof(command_line_options));
    options->first_file = strdup(first_file);
    options->second_file = strdup(second_file);
    return options;
}

command_line_options *command_line_options_create_default()
{
    command_line_options *options = malloc(sizeof(command_line_options));
    options->first_file = NULL;
    options->second_file = NULL;
    return options;
}

void command_line_options_destroy(command_line_options *options)
{
    free(options->first_file);
    free(options->second_file);
    free(options);
}

command_line_options *process_command_line_arguments(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("[Usage] diff.exe [options] [file_a] [file_b]\n");
        exit(1);
    }

    // This will not crash because we are guaranteed to have >= 3 arguments.
    // The last two arguments will always be the files to process.
    command_line_options *options = command_line_options_create_default();
    options->first_file = strdup(argv[argc - 2]);
    options->second_file = strdup(argv[argc - 1]);

    //     for (int i = argc-1; i > 1; i--)
    //     {
    //         options->second_file = strdup
    //     }
    return options;
}

int similarity_last_line(similarity *sim)
{
    return sim->line_number + sim->total_lines_matched;
}

int discrepancy_last_line(discrepancy *disc)
{
    return disc->line_number + disc->total_lines;
}

void print_diff(file *first, file *second, similarity_graph *sim_graph, discrepancy_graph *disc_graph)
{
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
                file_print_similarity(second, sim);
                line_number += sim->total_lines_matched;
                sim = sim->next;
            }

            else
            {
                file_print_discrepancy(first, second, disc);
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
            file_print_similarity(second, sim);
            line_number += sim->total_lines_matched;
            sim = sim->next;
        }

        // Only discrepancies left
        else if (sim == NULL)
        {
            file_print_discrepancy(first, second, disc);
            line_number += disc->total_lines;
            disc = disc->next;
        }
    }
}

int main(int argc, char *argv[])
{
    command_line_options *options = process_command_line_arguments(argc, argv);

    file *first = file_open(options->first_file);
    file *second = file_open(options->second_file);

    command_line_options_destroy(options);

    similarity_graph *graph = generate_graph(first, second);
    similarity_graph_print(first, second, graph);
    print_seperator();
    // similarity_graph_print_alt(first, second, graph);

    discrepancy_graph *discrepancy_graph = generate_discrepancy_graph(first, second, graph);
    discrepancy_graph_print(first, second, discrepancy_graph);

    print_seperator();
    print_diff(first, second, graph, discrepancy_graph);

    discrepancy_graph_destroy(discrepancy_graph);
    similarity_graph_destroy(graph);

    file_close(first);
    file_close(second);
    return 0;
}