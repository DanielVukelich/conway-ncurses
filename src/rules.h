#ifndef RULEPARSE_H
#define RULEPARSE_H

#include <stdbool.h>

//A cell can have 0 to 8 living neighbors
#define NUM_RULES 9

enum rule_type{
    DIE = 0b00,
    KEEP_ALIVE = 0b10,
    BE_BORN = 0b01
};

typedef struct ruleset_t{
    enum rule_type rules[NUM_RULES];
} rule_set;

int parse_rules(rule_set* rules, char* rule_string);
bool next_cell_state(rule_set* rules, bool cell_state, int num_neighbours);

#endif
