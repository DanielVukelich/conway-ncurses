#ifndef GAMEFIELD_H
#define GAMEFIELD_H

#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "bit_accessor.h"
#include "rules.h"

//The lower the seed rate, the more cells will be seeded as "alive" when the program starts
#define SEED_RATE 2
//As per the Life 1.05 file specification, a line is 80 chars long + terminator char
#define FILE_LINE_LENGTH 81
//The default rules for conway's game of life:  A live cell stays alive with 2 or 3 live
//neighbours, and a dead cell comes alive with 3 live neighbours
#define DEFAULT_RULES "23/3"
//The speed at which the simulation is run
#define DEFAULT_SPEED 250

typedef struct field_data_t{
    bit_accessor* buffer_r;
    bit_accessor* buffer_w;

    unsigned int field_len;
    unsigned int size_x;
    unsigned int size_y;
    bool edge_wrap;
    rule_set rules;
} field_data;

int init_field(field_data* field, int width, int height, int seed_rate, bool edge_wrap, char* rules);
int init_field_file(field_data* field, FILE* fp, int width, int height, bool edge_wrap, char* rules);
void free_field(field_data* field);
void update_and_swap_fields(field_data* field);
bool get_cell(field_data* field, unsigned int offset);

#endif
