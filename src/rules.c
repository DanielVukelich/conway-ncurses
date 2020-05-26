#include <ctype.h>

#include "rules.h"
#include "errcode.h"

#define RULE_SEPARATOR_CHAR '/'
#define RULE_SEPARATOR_CONST -1
#define CHAR_IGNORE_CONST -2
#define INVALID_CHAR -3

int char_to_int(char c){
    switch(c){
    case '0':
        return 0;
    case '1':
        return 1;
    case '2':
        return 2;
    case '3':
        return 3;
    case '4':
        return 4;
    case '5':
        return 5;
    case '6':
        return 6;
    case '7':
        return 7;
    case '8':
        return 8;
    case RULE_SEPARATOR_CHAR:
        return RULE_SEPARATOR_CONST;
    }

    if(isspace(c))
        return CHAR_IGNORE_CONST;

    return INVALID_CHAR;
}

int parse_rules(rule_set* rule_set, char* rule_string){
    //Fill the rules array with default values
    for(int i = 0; i < NUM_RULES; ++i){
        rule_set->rules[i] = DIE;
    }
    //Any numbers we read before a '/' are how many neighbors a live cell needs to stay alive
    enum rule_type rule_bucket = KEEP_ALIVE;

    for(char* p = rule_string; *p != '\0'; ++p){
        int parsed_char = char_to_int(*p);

        //Do not accept unexpected characters
        if(parsed_char == INVALID_CHAR)
            return RULE_PARSE_FAIL;

        //Ignore whitespace
        if(parsed_char == CHAR_IGNORE_CONST)
            continue;

        if(parsed_char == RULE_SEPARATOR_CONST){
            //After the '/', a number means how many neighbors a cell must have to be born
            if(rule_bucket == KEEP_ALIVE)
                rule_bucket = BE_BORN;
            //We return an error code if there is more than one '/' in the string
            else
                return RULE_PARSE_FAIL;
        }else{
            rule_set->rules[parsed_char] |= rule_bucket;
        }
    }
    return NO_ERR;
}

bool next_cell_state(rule_set* rule_set, bool cell_state, int num_neighbours){
    if(num_neighbours < 0 || num_neighbours >= NUM_RULES)
        return false;

    enum rule_type to_apply = rule_set->rules[num_neighbours];
    if(to_apply == DIE)
        return false;
    if(to_apply & KEEP_ALIVE && cell_state)
        return true;
    if(to_apply & BE_BORN && !cell_state)
        return true;

    return false;
}
