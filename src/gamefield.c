#include "gamefield.h"
#include "errcode.h"

int count_neighbours(field_data field, unsigned int offset);
bool read_cell(field_data field, unsigned int size_x, unsigned int y);
bool apply_rules(field_data field, bool state, int neighbours);
void set_primary_cell(field_data field, unsigned int x, unsigned int y, bool val);
void set_buffer_cell(field_data field, unsigned int offset, bool val);
void toggle_buffer_cell(field_data field, unsigned int offset);

int count_neighbours(field_data field, unsigned int offset){
    int count = 0;
    int x = offset % field.size_x;
    int y = offset / field.size_x;
    if(read_cell(field, x - 1, y - 1))
        ++count;
    if(read_cell(field, x - 1, y))
        ++count;
    if(read_cell(field, x - 1, y + 1))
        ++count;
    if(read_cell(field, x, y - 1))
        ++count;
    if(read_cell(field, x, y + 1))
        ++count;
    if(read_cell(field, x + 1, y - 1))
        ++count;
    if(read_cell(field, x + 1, y))
        ++count;
    if(read_cell(field, x + 1, y + 1))
        ++count;
    return count;
}

void update_and_swap_fields(field_data* field){
    for(unsigned int offset = 0; offset < field->buff_len; ++offset){
        unsigned int neigh = count_neighbours(*field, offset);
        bool state = get_primary_cell(*field, offset);
        bool nextState = apply_rules(*field, state, neigh);
        set_buffer_cell(*field, offset, nextState);
    }
    bool* temp = field->buffer_r;
    field->buffer_r = field->buffer_w;
    field->buffer_w = temp;
    return;
}

void clear_field(field_data *field){
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
        for(int i = 0; i < field->buff_len; ++i){
            field->buffer_r[i] = !(rand() % seed_rate);
        }
    }else{
        memset(field->buffer_r, false, field->buff_len);
    }
}

int init_field(field_data *field, int width, int height, int seed_rate, bool edge_wrap, char* rules){
    srand(time(0));
    if(!rules)
        rules = DEFAULT_RULES;

    unsigned int buff_len = width * height;
    size_t buff_size = sizeof(_Bool) * buff_len;

    field->buff_len = buff_len;
    field->edge_wrap = edge_wrap;
    field->size_x = width;
    field->size_y = height;

    field->buffer_r = malloc(buff_size);
    if(field->buffer_r == NULL)
        return OUT_OF_MEM;

    field->buffer_w = malloc(buff_size);
    if(field->buffer_w == NULL)
        return OUT_OF_MEM;

    seed_field(field, seed_rate);

    if(parse_rules(field, rules)){
        clear_field(field);
        return RULE_PARSE_FAIL;
    }
    return NO_ERR;
}


int init_field_file(field_data *field, FILE *fp, int width, int height, bool edge_wrap, char* rules){
    if(fp == NULL)
        return FILE_NOT_FOUND;


    enum errcode status = init_field(field, width, height, 0, edge_wrap, rules);

    if(status != NO_ERR)
        return status;

    char inputbuffer[FILE_LINE_LENGTH];
    int xstart = width / 2;
    int ystart = height / 2;
    int y = ystart;
    int linesread = 0;
    char* correctheader = "#Life 1.05";

    if(fgets(inputbuffer, FILE_LINE_LENGTH, fp)){
        if(strncmp(inputbuffer, correctheader, 10)){
            clear_field(field);
            return FILE_FORMAT_UNEXP;
        }

    }else{
        clear_field(field);
        return FILE_FORMAT_UNEXP;
    }

    bool readFileRules = false;
    bool readArgRules = false;
    bool readP = false;

    if(rules){
        if(parse_rules(field, rules)){
            clear_field(field);
            return RULE_PARSE_FAIL;
        }
        readArgRules = true;
    }

    while(fgets(inputbuffer, FILE_LINE_LENGTH, fp) && y < field->size_y){
        if(inputbuffer[0] == '#'){
            if(inputbuffer[1] == 'R'){
                if(readFileRules){
                    clear_field(field);
                    return FILE_DUPE_ATTR;
                }
                if(readArgRules){
                    free(field->rules.born_rules);
                }
                if(parse_rules(field, inputbuffer + 2)){
                    clear_field(field);
                    return RULE_PARSE_FAIL;
                }
                readFileRules = true;
            }
            else if(inputbuffer[1] == 'N'){
                if(readFileRules){
                    clear_field(field);
                    return FILE_DUPE_ATTR;
                }
                if(readArgRules){
                    free(field->rules.born_rules);
                }
                parse_rules(field, DEFAULT_RULES);
                readFileRules = true;
            }
            else if(inputbuffer[1] == 'D')
                continue;
            else if(inputbuffer[1] == 'P'){

                if(readP){
                    clear_field(field);
                    return FILE_DUPE_ATTR;
                }
                readP = true;

                char *start1, *end1, *start2, *end2;
                for(start1 = &inputbuffer[2]; isspace(*start1); ++start1);
                for(end1 = start1; !isspace(*end1); ++end1);
                *end1 = '\0';
                for(start2 = end1 + 1; isspace(*start2); ++start2);
                for(end2 = start2; !isspace(*end2); ++end2);
                *end2 = '\0';

                int xoff = atoi(start1);
                int yoff = atoi(start2);
                xstart += xoff;
                if(xstart < 0)
                    xstart = 0;
                ystart += yoff;
                if(ystart < 0)
                    ystart = 0;
                y = ystart;
            }
        }else if(inputbuffer[0] == '.' || inputbuffer[0] == '*' || inputbuffer[0] == '\n'){
            bool readingline = true;
            for(unsigned int x = 0; x < field->size_x; ++x){
                if(inputbuffer[x] == '\n' || inputbuffer[x] == '\0'){
                    readingline = false;
                }
                if(!readingline || inputbuffer[x] == '.'){
                    //Fill the cell with 0
                    set_primary_cell(*field, x + xstart, y, false);
                }else if(inputbuffer[x] == '*'){
                    //Cell is alive
                    set_primary_cell(*field, x + xstart, y, true);
                }else{
                    //Invalid format
                    clear_field(field);

                    puts(inputbuffer);
                    return FILE_LAYOUT_MALFORM;
                }
            }
            ++y;
        }else{
            //Malformed input file
            clear_field(field);
            return FILE_TAGS_MALFORM;
        }
        ++linesread;
    }

    for(unsigned int offset = y * field->size_x; offset < field->size_y * field->size_x; ++offset)
        field->buffer_r[offset] = false;

    //  If no ruleset has been defined in either the file or the command line args, use the default
    if(!readFileRules && !readArgRules)
        parse_rules(field, DEFAULT_RULES);

    return NO_ERR;
}

bool read_cell(field_data field, unsigned int x, unsigned int y){
    if(x >= field.size_x || y >= field.size_y){
        if(!field.edge_wrap)
            return false;
        x = (x + field.size_x) % field.size_x;
        y = (y + field.size_y) % field.size_y;
    }
    unsigned int offset = x + (field.size_x * y);
    return field.buffer_r[offset];
}

inline void set_primary_cell(field_data field, unsigned int x, unsigned int y, bool val){
    unsigned int offset = x + field.size_x * y;
    field.buffer_r[offset] = val;
}

inline void set_buffer_cell(field_data field, unsigned int offset, bool val){
    field.buffer_w[offset] = val;
}

inline void toggle_buffer_cell(field_data field, unsigned int offset){
    field.buffer_w[offset] = !field.buffer_r[offset];
}

inline bool get_primary_cell(field_data field, unsigned int offset){
    return field.buffer_r[offset];
}
