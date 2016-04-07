#ifndef GAMEFIELD_H
#define GAMEFIELD_H

#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

//The lower the seed rate, the more cells will be seeded as "alive" when the program starts
#define SEED_RATE 2
//As per the Life 1.05 file specification, a line is 80 chars long + terminator char
#define FILE_LINE_LENGTH 81
//The default rules for conway's game of life:  A live cell stays alive with 2 or 3 live 
//neighbours, and a dead cell comes alive with 3 live neighbours
#define DEFAULT_RULES "23/3"
//The speed at which the simulation is run
#define DEFAULT_SPEED 250

typedef struct ruleset_t{
  int* born_rules;
  int* keepalive_rules;
  int numBornRules;
  int numKeepaliveRules;
} ruleset;

typedef struct field_data_t{
  bool* primary_field_ptr;
  bool* buffer_field_ptr;
  unsigned int x;
  unsigned int y;
  bool infinite;
  ruleset rules;
} field_data;

int init_field(field_data* field, int seed_rate, bool infinite, char* rules);
int init_field_file(field_data* field, FILE* fp, bool infinite, char* rules);
int parse_rules(field_data* field, char* toparse);
void update_and_swap_fields(field_data* field);
void clear_field(field_data* field);
bool get_primary_cell(field_data field, unsigned int offset);

#endif
