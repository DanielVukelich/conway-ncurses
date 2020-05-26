#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "bit_accessor.h"
#include "rules.h"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

typedef struct unit_test_t{
    const char* Description;
    int (*Test) (void);
} unit_test;

char* bool_2_str(bool value){
    return value ? "true" : "false";
}

int test_zero_initialized(){
    bit_accessor accessor;
    bit_accessor* ba = &accessor;

    unsigned int numbits = 65;

    init_accessor(ba, numbits);

    for(unsigned int i = 0; i < numbits; ++i){
        if(get_bit(ba,1)){
            printf("Expected bit %i to be 0, but was 1\n", i);
            free_accessor(ba);
            return 1;
        }
    }
    free_accessor(ba);
    return 0;
}

int test_bit_set_function(){
    bit_accessor accessor;
    bit_accessor* ba = &accessor;

    unsigned int numbits = 65;

    init_accessor(ba, numbits);

    for(unsigned int i = 0; i < numbits; ++i){
        set_bit(ba, i, true);
    }

    for(unsigned int i = 0; i < numbits; ++i){
        if(!get_bit(ba,1)){
            printf("Expected bit %i to be 1, but was 0\n", i);
            free_accessor(ba);
            return 1;
        }
    }
    free_accessor(ba);
    return 0;
}

int test_bit_toggle_onepass(){
    bit_accessor accessor;
    bit_accessor* ba = &accessor;

    unsigned int numbits = 65;

    init_accessor(ba, numbits);
    for(unsigned int i = 0; i < numbits; ++i){
        toggle_bit(ba, i);
        if(!get_bit(ba,i)){
            printf("Expected bit %i to be 1, but was 0\n", i);
            free_accessor(ba);
            return 1;
        }
    }

    free_accessor(ba);
    return 0;
}

int test_bit_toggle_twopass(){
    bit_accessor accessor;
    bit_accessor* ba = &accessor;

    unsigned int numbits = 65;

    init_accessor(ba, numbits);
    for(unsigned int i = 0; i < numbits; ++i){
        toggle_bit(ba, i);
    }

    for(unsigned int i = 0; i < numbits; ++i){
        if(!get_bit(ba,i)){
            printf("Expected bit %i to be 1, but was 0\n", i);
            free_accessor(ba);
            return 1;
        }
    }
    free_accessor(ba);
    return 0;
}

int test_rule_parsing(){

    struct test_data_t{
        int num_neighbours;
        bool current_state;
        bool expected_next_state;
    };

    struct test_data_t tests[] = {
        {0, false, false},
        {1, false, false},
        {-2, false, false},
        {10, true, false},
        {2, true, true},
        {3, true, true},
        {3, false, true},
        {2, false, false},
        {2, false, false},
        {999, false, false} // 999 neighbours symbolizes end
    };

    rule_set* r = malloc(sizeof(rule_set));

    char* rule_string = "23/3";
    int status = parse_rules(r, rule_string);
    if(status){
        printf("Expected nonzero status when parsing rule string \'%s\'", rule_string);
        free(r);
        return status;
    }

    int i = 0;
    bool exit_err = false;
    for(struct test_data_t* t = tests; t->num_neighbours != 999; ++t, ++i){
        bool expected = t->expected_next_state;
        bool actual = next_cell_state(r, t->current_state, t->num_neighbours);

        if(expected != actual){
            printf("Given rules \'%s\' Expected a %s cell with %i neighbours to transition to %s but got %s\n", rule_string, bool_2_str(t->current_state), t->num_neighbours, bool_2_str(expected), bool_2_str(actual));
            exit_err = true;
        }
    }

    free(r);
    return exit_err ? 1 : 0;
}

unit_test tests[] = {
    {"Bit accessor initializes to all zero bits", &test_zero_initialized},
    {"Bit accessor writes the same bits it reads", &test_bit_set_function},
    {"Flipping a zero bit and immediately reading it results in 1", &test_bit_toggle_onepass },
    {"Flipping a whole field of zero bits and then reading them results in all 1", &test_bit_toggle_twopass},
    {"Parsing of standard \"23/3 ruleset\"", &test_rule_parsing},
    {NULL, NULL}
};

int main(){
    printf(ANSI_COLOR_YELLOW "Running test suite\n" ANSI_COLOR_RESET);
    int failures = 0;
    int pass = 0;
    int total = 0;

    for(unit_test* t = tests; t->Description != NULL; ++t){
        ++total;
        printf(ANSI_COLOR_BLUE "Testing: %s\n" ANSI_COLOR_RESET, t->Description);
        if(t->Test()){
            printf(ANSI_COLOR_RED "FAIL\n" ANSI_COLOR_RESET);
            ++failures;
        }
        else{
            printf(ANSI_COLOR_GREEN "PASS\n" ANSI_COLOR_RESET);
            ++pass;
        }
    }

    if(failures == 0)
        printf("Test suite complete.  " ANSI_COLOR_GREEN "%i tests passed out of %i tests run\n" ANSI_COLOR_RESET, pass, total);
    else
        printf("Test suite complete.  " ANSI_COLOR_RED "%i tests failed out of %i tests run\n" ANSI_COLOR_RESET, failures, total);

    return failures;
}
