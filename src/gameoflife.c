#include <ncurses.h>
#include <getopt.h>
#include "gamefield.h"

typedef struct arg_t{
  char* infile;
  char* ruleset;
  int seed_rate;
  int game_speed;
  bool widescreen;
  bool infinite;
  bool paused;
} arg_data;

void print_error(int err, char** argv);
void ncurses_init(field_data* field, bool widescreen, int speed);
void draw_and_refresh(field_data field, bool widescreen);
int get_opts(arg_data* args, int argc, char** argv);

int main(int argc, char** argv){

  arg_data args = {NULL, NULL, SEED_RATE, DEFAULT_SPEED, false, false, false};
  field_data field;  
  bool running = true;
  bool step = false;
  
  int err = get_opts(&args, argc, argv);
  ncurses_init(&field, args.widescreen, args.game_speed);
  bool paused = args.paused;

  if(!err){
    if(args.infile)
      err = init_field_file(&field, fopen(args.infile, "r"), args.infinite, args.ruleset);
    else
      err = init_field(&field, args.seed_rate, args.infinite, args.ruleset);
  }

  if(!err){    
    unsigned int generations = 0;
    draw_and_refresh(field, args.widescreen);

    while(running){
      int ch = getch();
      running = (ch != 'q');
      paused = (paused ^ (ch == ' '));
      step = (paused && (ch == 's'));
      if(!paused || step){
	update_and_swap_fields(&field);
	draw_and_refresh(field, args.widescreen);
	++generations;
	step = false;
      }
    }

    endwin();
    clear_field(&field);
    printf("%i generation%s simulated\n", generations, (generations != 1) ? "s" : "");

    return 0;
    
  }
 
  endwin();
  print_error(err, argv);
  
  return 1;
}

void draw_and_refresh(field_data field, bool widescreen){  
  for(unsigned int offset = 0; offset < (field.x * field.y); ++offset){
    int cell = 32 | (get_primary_cell(field, offset) ? 0 : A_REVERSE);
    unsigned int x = offset % field.x;
    unsigned int y = (offset / field.x) % field.y;
    if(widescreen)
      mvaddch(y, x, cell);
    else{
      mvaddch(y, 2 * x, cell);
      mvaddch(y, 2 * x + 1, cell);
    }
  }
  refresh();
}

void ncurses_init(field_data* field, bool widescreen, int speed){
  initscr();
  raw();
  keypad(stdscr, true);
  noecho();
  curs_set(0);
  getmaxyx(stdscr, field->y, field->x);
  timeout(speed);
  if(!widescreen)
    field->x /= 2;
  return;
}

int get_opts(arg_data* args, int argc, char** argv){
  static struct option long_options[] = {
    {"file", required_argument, 0, 'f'},
    {"seed", required_argument, 0, 's'},
    {"rule", required_argument, 0, 'r'},
    {"widescreen", no_argument, 0, 'w'},
    {"infinite", no_argument, 0, 'i'},
    {"pause", no_argument, 0, 'p'},
    {"time", required_argument, 0, 't'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
  };
  int option_index = 0;
  int argres = 0;
  while(1){

    argres = getopt_long(argc, argv, "f:s:r:whipt:", long_options, &option_index);

    if(argres == -1)
      break;

    switch(argres){
    case 'i':
      args->infinite = true;
      break;
    case 't':
      args->game_speed = atoi(optarg);
      if(args->game_speed < 1)
	return 1;
      break;
    case 'h':
      return 1;
    case 'f':
      args->infile = optarg;
      break;
    case 'w':
      args->widescreen = true;
      break;
    case 'r':
      args->ruleset = optarg;
      break;
    case 's':
      args->seed_rate = atoi(optarg);
      if(args->seed_rate < 1)
	return 1;
      break;
    case 'p':
      args->paused = true;
      break;
    default:
      return 1;
    }
  }

  return 0;
}

void print_error(int err, char** argv){
  switch(err){
  case 1:
    printf("Usage:\n%s --file Life_1.05_file --seed seed_rate_number --rule ruleset_string --widescreen --infinite --time time_speed --pause\n", argv[0]);
    puts("To control the game, press 'q' to exit, space to pause, and 's' while paused to advance one generation");
    return;
  case -1:
    puts("Could not find the specified file");
    return;
  case -2:
    puts("Specified file had layout description line(s) with invalid format");
    return;
  case -3:
    puts("Specified file had improperly formatted tags");
    return;
  case -4:
    puts("Life 1.05 file used a currently unsupported tag");
    return;
  case -5:
    puts("Specified file is not Life 1.05 format");
    return;
  case -6:
    puts("Specified file had duplicate attribute tags");
    return;
  case -7:
    puts("Specified ruleset is improperly formatted (check #R tag in file or the program arguments)");
    return;
  }
}


