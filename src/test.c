#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include "bit_accessor.h"

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

unit_test tests[] = {
    {"Bit accessor initializes to all zero bits", &test_zero_initialized},
    {"Bit accessor writes the same bits it reads", &test_bit_set_function},
    {"Flipping a zero bit and immediately reading it results in 1", &test_bit_toggle_onepass },
    {"Flipping a whole field of zero bits and then reading them results in all 1", &test_bit_toggle_twopass},
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
