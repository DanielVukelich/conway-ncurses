#include "gamefield.h"

unsigned int count_neighbours(field_data field, unsigned int offset);
bool get_primary_cell_xy(field_data field, unsigned int x, unsigned int y);
bool apply_rules(field_data field, bool state, unsigned int neighbours);
void set_primary_cell(field_data field, unsigned int x, unsigned int y, bool val);
void set_buffer_cell(field_data field, unsigned int offset, bool val);
void toggle_buffer_cell(field_data field, unsigned int offset);

unsigned int count_neighbours(field_data field, unsigned int offset){
  unsigned int count = 0;
  unsigned int x = offset % field.x;
  unsigned int y = offset / field.x;
  if(get_primary_cell_xy(field, x - 1, y - 1))
    ++count;
  if(get_primary_cell_xy(field, x - 1, y))
    ++count;
  if(get_primary_cell_xy(field, x - 1, y + 1))
    ++count;
  if(get_primary_cell_xy(field, x, y - 1))
    ++count;
  if(get_primary_cell_xy(field, x, y + 1))
    ++count;
  if(get_primary_cell_xy(field, x + 1, y - 1))
    ++count;
  if(get_primary_cell_xy(field, x + 1, y))
    ++count;
  if(get_primary_cell_xy(field, x + 1, y + 1))
    ++count;
  return count;
}

void update_and_swap_fields(field_data* field){
  for(unsigned int offset = 0; offset < (field->x * field->y); ++offset){
      unsigned int neigh = count_neighbours(*field, offset);
      bool state = get_primary_cell(*field, offset);
      bool nextState = apply_rules(*field, state, neigh);
      set_buffer_cell(*field, offset, nextState);
  }
  bool* temp = field->primary_field_ptr;
  field->primary_field_ptr = field->buffer_field_ptr;
  field->buffer_field_ptr = temp;
  return;
}

void clear_field(field_data *field){
  free(field->primary_field_ptr);
  free(field->buffer_field_ptr);
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
      return 1;
  }
  for(int i = 0; i < nKr; ++i){
    int toadd = start1[i] - '0';
    if(toadd >= 0 && toadd <= 8)
      field->rules.keepalive_rules[i] = toadd;
    else
      return 1;
  }
  return 0;
}

bool apply_rules(field_data field, bool state, unsigned int neighbours){
  if(!state){
    for(int i = 0; i < field.rules.numBornRules; ++i){
      if(neighbours == field.rules.born_rules[i])
	return true;
    }
    return false;
  }else{
    bool toReturn = false;
    for(int i = 0; i < field.rules.numKeepaliveRules; ++i){
      if(neighbours == field.rules.keepalive_rules[i])
	toReturn = true;
    }
    return toReturn;
  }
}

int init_field(field_data *field, int seed_rate, bool infinite, char* rules){
  srand(time(0));
  if(!rules)
    rules = DEFAULT_RULES;
  field->infinite = infinite;
  field->primary_field_ptr = malloc(sizeof(_Bool) * field->x * field->y);
  field->buffer_field_ptr = malloc(sizeof(_Bool) * field->x * field->y);
  for(unsigned int off = 0; off < (field->x * field->y); ++off){
    field->primary_field_ptr[off] = !(rand() % seed_rate);
  }
  if(parse_rules(field, rules)){
    clear_field(field);
    return -7;
  }
  return 0;
}

int init_field_file(field_data *field, FILE *fp, bool infinite, char* rules){
  if(fp == NULL)
    return -1;
  int xstart = field->x / 2;
  int ystart = field->y / 2;
  size_t fsize = sizeof(_Bool) * field->x * field->y;

  field->infinite = infinite;
  field->primary_field_ptr = malloc(fsize);
  field->buffer_field_ptr = malloc(fsize);
  memset(field->primary_field_ptr, false, fsize);

  char inputbuffer[FILE_LINE_LENGTH];
  unsigned int y = ystart;
  int linesread = 0;
  char* correctheader = "#Life 1.05";

  if(fgets(inputbuffer, FILE_LINE_LENGTH, fp)){
    if(strncmp(inputbuffer, correctheader, 10)){
      clear_field(field);
      return -5;
    }
    
  }else{
    clear_field(field);
    return -5;
  }
  
  bool readFileRules = false;
  bool readArgRules = false;
  bool readP = false;

  if(rules){
    if(parse_rules(field, rules)){
      clear_field(field);
      return -7;
    }
    readArgRules = true;
  }
  
  while(fgets(inputbuffer, FILE_LINE_LENGTH, fp) && y < field->y){
    if(inputbuffer[0] == '#'){
      if(inputbuffer[1] == 'R'){
	if(readFileRules){
	  clear_field(field);
	  return -6;
	}
	if(readArgRules){
	  free(field->rules.born_rules);
	}
	if(parse_rules(field, inputbuffer + 2)){
	  clear_field(field);
	  return -7;
	}
	readFileRules = true;
      }
      else if(inputbuffer[1] == 'N'){
	if(readFileRules){
	  clear_field(field);
	  return -6;
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
	  return -6;
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
      for(unsigned int x = 0; x < field->x; ++x){
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
	  exit(0);
	  return -2;
	}
      }
      ++y;
    }else{
      //Malformed input file
      clear_field(field);
      return -3;
    }
    ++linesread;
  }

  for(unsigned int offset = y * field->x; offset < field->y * field->x; ++offset)
    field->primary_field_ptr[offset] = false;

  //  If no ruleset has been defined in either the file or the command line args, use the default
  if(!readFileRules && !readArgRules)
    parse_rules(field, DEFAULT_RULES);

  return 0;
}

bool get_primary_cell_xy(field_data field, unsigned int x, unsigned int y){
  if(x >= field.x || y >= field.y){
    if(!field.infinite)
      return false;
    x = (x + field.x) % field.x;
    y = (y + field.y) % field.y;
  }
  unsigned int offset = x + (field.x * y);
  return field.primary_field_ptr[offset];
}

inline void set_primary_cell(field_data field, unsigned int x, unsigned int y, bool val){
  unsigned int offset = x + field.x * y;
  field.primary_field_ptr[offset] = val;
}

inline void set_buffer_cell(field_data field, unsigned int offset, bool val){
  field.buffer_field_ptr[offset] = val;
}

inline void toggle_buffer_cell(field_data field, unsigned int offset){
  field.buffer_field_ptr[offset] = !field.primary_field_ptr[offset];
}

inline bool get_primary_cell(field_data field, unsigned int offset){
  return field.primary_field_ptr[offset];
}
