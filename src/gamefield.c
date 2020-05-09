#include <limits.h>

#include "gamefield.h"
#include "errcode.h"

enum cell_status{
    DEAD = 0,
    ALIVE,
    INVALID
};

const char LIVE_CELL = '*';
const char DEAD_CELL = '.';

char random_word(int seed_rate);
unsigned int num_words_for_field(unsigned int field_len);
void swap_buffers(field_data* field);
int count_neighbours(field_data* field, unsigned int offset);
bool apply_rules(field_data field, bool state, int neighbours);
void set_cell(field_data* field, unsigned int offset, bool val);
void toggle_cell(field_data* field, unsigned int offset);
unsigned int relative_offset(field_data* field, unsigned int offset, int rel_x, int rel_y);
unsigned int next_newline_from(field_data* field, unsigned int offset);
bool has_prefix(char* string, const char* prefix);
bool is_line_end(char* string);
enum cell_status parse_field_cell(char c);
unsigned int pattern_next_line(field_data* field, unsigned int pattern_cursor, unsigned int newline_offset);
bool get_cell_relative_offset(field_data* field, unsigned int offset, int rel_x, int rel_y);

int count_neighbours(field_data* field, unsigned int offset){
    int count = 0;
    for(int x_offset = -1; x_offset <= 1; ++x_offset){
        for(int y_offset = -1; y_offset <= 1; ++y_offset){
            //A cell is not its own neighbour
            if(x_offset == 0 && y_offset == 0)
                continue;
            if(get_cell_relative_offset(field, offset, x_offset, y_offset))
               ++count;
        }
    }
    return count;
}

bool get_cell_relative_offset(field_data* field, unsigned int offset, int rel_x, int rel_y){

    int new_x = rel_x + (offset % field->size_x);
    if(new_x < 0){
        if(!field->edge_wrap)
            return false;
        new_x += field->size_x;
    }else if((unsigned int) new_x >= field->size_x){
        if(!field->edge_wrap)
            return false;
        new_x %= field->size_x;
    }

    int new_y = rel_y + (offset / field->size_x);
    if(new_y < 0){
        if(!field->edge_wrap)
            return false;
        new_y += field->size_y;
    }else if((unsigned int) new_y >= field->size_y){
        if(!field->edge_wrap)
            return false;
        new_y %= field->size_y;
    }

    unsigned int offset_new = (new_y * field->size_x) + new_x;

    return get_cell(field, offset_new);
}

unsigned int relative_offset(field_data* field, unsigned int offset, int rel_x, int rel_y){
    int x_offset = rel_x;
    int y_offset = field->size_x * rel_y;
    int total_offset = x_offset + y_offset;
    return (unsigned int) (offset + total_offset);
}

unsigned int pattern_next_line(field_data* field, unsigned int pattern_cursor, unsigned int newline_offset){
    unsigned int offset_to_remove = (pattern_cursor % field->size_x);
    return (pattern_cursor + field->size_x - offset_to_remove + newline_offset);
}

void update_and_swap_fields(field_data* field){
    for(unsigned int offset = 0; offset < field->field_len; ++offset){
        unsigned int neigh = count_neighbours(field, offset);
        bool state = get_cell(field, offset);
        bool nextState = apply_rules(*field, state, neigh);
        set_cell(field, offset, nextState);
    }

    swap_buffers(field);
    return;
}

void free_field(field_data *field){
    free_accessor(field->buffer_r);
    free_accessor(field->buffer_w);
    free(field->buffer_r);
    free(field->buffer_w);
    free(field->rules.born_rules);
    free(field->rules.keepalive_rules);
}

int parse_rules(field_data* field, char* toparse){
    char *start1, *end1, *start2, *end2;
    for(start1 = toparse; isspace(*start1); ++start1);
    for(end1 = start1; *end1 != '/'; ++end1);
    int nKr = end1 - start1;
    for(start2 = end1 + 1; isspace(*start2); ++start2);
    for(end2 = start2; !isspace(*end2) && *end2 != '\0'; ++end2);
    int nBr = end2 - start2;
    field->rules.numBornRules = nBr;
    field->rules.numKeepaliveRules = nKr;

    field->rules.born_rules = malloc(sizeof(int) * nBr);
    field->rules.keepalive_rules = malloc(sizeof(int) * nKr);
    for(int i = 0; i < nBr; ++i){
        int toadd = start2[i] - '0';
        if(toadd >= 0 && toadd <= 8)
            field->rules.born_rules[i] = toadd;
        else
            return RULE_PARSE_FAIL;
    }
    for(int i = 0; i < nKr; ++i){
        int toadd = start1[i] - '0';
        if(toadd >= 0 && toadd <= 8)
            field->rules.keepalive_rules[i] = toadd;
        else
            return RULE_PARSE_FAIL;
    }
    return NO_ERR;
}

bool apply_rules(field_data field, bool state, int neighbours){
    int* applied_rules = state ? field.rules.keepalive_rules : field.rules.born_rules;
    int num_rules = state ? field.rules.numKeepaliveRules : field.rules.numBornRules;

    for(int i = 0; i < num_rules; ++i){
        if(neighbours == applied_rules[i])
            return true;
    }
    return false;
}

void seed_field(field_data* field, int seed_rate){
    if(seed_rate){
        for(unsigned int i = 0; i < field->field_len; ++i){
            bool rand_val = (rand() % seed_rate == 0);
            set_bit(field->buffer_r, i, rand_val);
        }
    }
}

int init_field(field_data *field, int width, int height, int seed_rate, bool edge_wrap, char* rules){
    srand(time(0));
    if(!rules)
        rules = DEFAULT_RULES;

    field->field_len = width * height;
    field->edge_wrap = edge_wrap;
    field->size_x = width;
    field->size_y = height;

    field->buffer_r = malloc(sizeof(bit_accessor));
    if(field->buffer_r == NULL)
        return OUT_OF_MEM;

    field->buffer_w = malloc(sizeof(bit_accessor));
    if(field->buffer_w == NULL)
        return OUT_OF_MEM;

    int status = init_accessor(field->buffer_r, field->field_len);
    if(status != NO_ERR)
        return status;

    status = init_accessor(field->buffer_w, field->field_len);
    if(status != NO_ERR)
        return status;

    seed_field(field, seed_rate);

    if(parse_rules(field, rules)){
        free_field(field);
        return RULE_PARSE_FAIL;
    }
    return NO_ERR;
}

bool has_prefix(char* string, const char* prefix){
    while(*prefix)
    {
        if(*prefix++ != *string++)
            return 0;
    }
    return 1;
}

bool is_line_end(char* string){
    if(string[0] == '\n')
        return true;
    if(string[0] == '\r' && string[1] == '\n')
       return true;
    return false;
}

enum cell_status parse_field_cell(char c){
    if(c == DEAD_CELL)
        return DEAD;
    if(c == LIVE_CELL)
        return ALIVE;
    return INVALID;
}

int init_field_file(field_data *field, FILE *fp, int width, int height, bool edge_wrap, char* rules){
    if(fp == NULL)
        return FILE_NOT_FOUND;


    enum errcode status = init_field(field, width, height, 0, edge_wrap, rules);

    if(status != NO_ERR)
        return status;

    char inputbuffer[FILE_LINE_LENGTH];
    unsigned int universal_center = relative_offset(field, 0, field->size_x / 2, field->size_y / 2);
    unsigned int pattern_cursor = UINT_MAX;
    unsigned int pattern_newline_offset = 0;

    if(fgets(inputbuffer, FILE_LINE_LENGTH, fp)){
        if(!has_prefix(inputbuffer, "#Life 1.05")){
            free_field(field);
            return FILE_FORMAT_UNEXP;
        }

    }else{
        free_field(field);
        return FILE_FORMAT_UNEXP;
    }

    bool readFileRules = false;
    bool readArgRules = false;

    if(rules){
        if(parse_rules(field, rules)){
            free_field(field);
            return RULE_PARSE_FAIL;
        }
        readArgRules = true;
    }

    while(fgets(inputbuffer, FILE_LINE_LENGTH, fp)){
        if(has_prefix(inputbuffer, "#R")){
            if(readFileRules){
                free_field(field);
                return FILE_DUPE_ATTR;
            }
            if(readArgRules){
                free(field->rules.born_rules);
            }
            if(parse_rules(field, inputbuffer + 2)){
                free_field(field);
                return RULE_PARSE_FAIL;
            }
            readFileRules = true;
        }else if(has_prefix(inputbuffer, "#N")){
            if(readFileRules){
                free_field(field);
                return FILE_DUPE_ATTR;
            }
            if(readArgRules){
                free(field->rules.born_rules);
            }
            parse_rules(field, DEFAULT_RULES);
            readFileRules = true;
        }else if(has_prefix(inputbuffer, "#D")){
            continue;
        }else if(has_prefix(inputbuffer, "#P")){
            char *start1, *end1, *start2, *end2;
            for(start1 = &inputbuffer[2]; isspace(*start1); ++start1);
            for(end1 = start1; !isspace(*end1); ++end1);
            *end1 = '\0';
            for(start2 = end1 + 1; isspace(*start2); ++start2);
            for(end2 = start2; !isspace(*end2); ++end2);
            *end2 = '\0';

            int x_off = atoi(start1);
            int y_off = atoi(start2);
            pattern_cursor = relative_offset(field, universal_center, x_off, y_off);
            pattern_newline_offset = pattern_cursor % field->size_x;
        }else if(pattern_cursor < field->field_len){
            for(char* p = inputbuffer; (*p != '\0') && (pattern_cursor < field->field_len); ++p){
                //Life 1.05 file format says to ignore empty lines
                //If p == inputbuffer and it's a line ending, then the line is empty and we do not increment the cursor.
                if(is_line_end(p)){
                    if(p != inputbuffer)
                        pattern_cursor = pattern_next_line(field, pattern_cursor, pattern_newline_offset);
                    break;
                }

                enum cell_status status = parse_field_cell(*p);
                if(status == INVALID){
                    //Invalid format
                    free_field(field);
                    return FILE_LAYOUT_MALFORM;
                }
                set_cell(field, pattern_cursor, (bool) status);
                ++pattern_cursor;
            }
        }
    }

    //If no ruleset has been defined in either the file or the command line args, use the default
    if(!readFileRules && !readArgRules)
        parse_rules(field, DEFAULT_RULES);

    swap_buffers(field);
    return NO_ERR;
}

bool get_cell(field_data* field, unsigned int offset){
    if(offset >= field->field_len){
        return false;
    }
    return get_bit(field->buffer_r, offset);
}

inline void swap_buffers(field_data* field){
    bit_accessor* temp = field->buffer_r;
    field->buffer_r = field->buffer_w;
    field->buffer_w = temp;
}

inline void set_cell(field_data* field, unsigned int offset, bool val){
    set_bit(field->buffer_w, offset, val);
}

inline void toggle_buffer_cell(field_data* field, unsigned int offset){
    toggle_bit(field->buffer_w, offset);
}
