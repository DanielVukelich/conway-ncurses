#ifdef DEBUG
#include <sys/prctl.h>
#include <sanitizer/asan_interface.h>
#endif

#include <ncurses.h>
#include <getopt.h>

#include "gamefield.h"
#include "errcode.h"

typedef struct arg_t{
  char* infile;
  char* ruleset;
  int seed_rate;
  int game_speed;
  bool widescreen;
  bool wrap_edges;
  bool paused;
  bool help;
} arg_data;

void print_error(int err, char** argv);
void ncurses_init(bool widescreen, int speed, int* x, int* y);
void draw_and_refresh(field_data field, bool widescreen);
int get_opts(arg_data* args, int argc, char** argv);

int main(int argc, char** argv){

#ifdef DEBUG
    //When debug mode enabled, allow any process to attach
    prctl(PR_SET_PTRACER, PR_SET_PTRACER_ANY);
    //Write address sanitizer errors to log file
    __sanitizer_set_report_path("asan.log");
#endif

  arg_data args = {NULL, NULL, SEED_RATE, DEFAULT_SPEED, false, false, false};
  field_data field;
  bool running = true;
  bool step = false;

  int err = get_opts(&args, argc, argv);
  int max_x, max_y;
  ncurses_init(args.widescreen, args.game_speed, &max_x, &max_y);
  bool paused = args.paused;

  if(!err){
    if(args.infile)
        err = init_field_file(&field, fopen(args.infile, "r"), max_x, max_y, args.wrap_edges, args.ruleset);
    else
        err = init_field(&field, max_x, max_y, args.seed_rate, args.wrap_edges, args.ruleset);
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
    free_field(&field);
    printf("%i generation%s simulated\n", generations, (generations != 1) ? "s" : "");

    return NO_ERR;

  }

  endwin();
  print_error(err, argv);

  if(args.help)
    return NO_ERR;

  return EXIT_ERR;
}

void draw_and_refresh(field_data field, bool widescreen){
  for(unsigned int offset = 0; offset < field.buff_len; ++offset){
    int cell = 32 | (get_cell(&field, offset) ? 0 : A_REVERSE);
    unsigned int x = offset % field.size_x;
    unsigned int y = (offset / field.size_x) % field.size_y;
    if(widescreen)
      mvaddch(y, x, cell);
    else{
      mvaddch(y, 2 * x, cell);
      mvaddch(y, 2 * x + 1, cell);
    }
  }
  refresh();
}

void ncurses_init(bool widescreen, int speed, int* scr_x, int* scr_y){
  initscr();
  raw();
  keypad(stdscr, true);
  noecho();
  curs_set(0);
  *scr_x = widescreen ? getmaxx(stdscr) : getmaxx(stdscr) / 2;
  *scr_y = getmaxy(stdscr);
  timeout(speed);
}

int get_opts(arg_data* args, int argc, char** argv){
  static struct option long_options[] = {
    {"file", required_argument, 0, 'f'},
    {"seed", required_argument, 0, 's'},
    {"rule", required_argument, 0, 'r'},
    {"widescreen", no_argument, 0, 'w'},
    {"edge-wrap", no_argument, 0, 'e'},
    {"pause", no_argument, 0, 'p'},
    {"time", required_argument, 0, 't'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
  };
  int option_index = 0;
  int argres = 0;
  while(1){

    argres = getopt_long(argc, argv, "f:s:r:whept:", long_options, &option_index);

    if(argres == -1)
      break;

    switch(argres){
    case 'e':
      args->wrap_edges = true;
      break;
    case 't':
      args->game_speed = atoi(optarg);
      if(args->game_speed < 1)
	return 1;
      break;
    case 'h':
      args->help = true;
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
      return PRINT_HELP;
    }
  }

  return NO_ERR;
}

void print_error(int err, char** argv){
  switch(err){
  case PRINT_HELP:
    printf("\nUsage:\n %s [args]\n\nPossible arguments:\n --file, -f\t\tLife_1.05_file\n --seed, -s\t\tseed_rate_number\n --rule, -r\t\truleset_string\n --widescreen, -w\n --edge-wrap, -e\n --time, -t\t\ttime_speed\n --pause, -p\n --help, -h\n", argv[0]);
    puts("\nTo control the game, press 'q' to exit, space to pause, and 's' while paused to advance one generation.");
    return;
  case FILE_NOT_FOUND:
    puts("Could not find the specified file");
    return;
  case FILE_LAYOUT_MALFORM:
    puts("Specified file had layout description line(s) with invalid format");
    return;
  case FILE_TAGS_MALFORM:
    puts("Specified file had improperly formatted tags");
    return;
  case FILE_TAG_UNSUPP:
    puts("Life 1.05 file used a currently unsupported tag");
    return;
  case FILE_FORMAT_UNEXP:
    puts("Specified file is not Life 1.05 format");
    return;
  case FILE_DUPE_ATTR:
    puts("Specified file had duplicate attribute tags");
    return;
  case RULE_PARSE_FAIL:
    puts("Specified ruleset is improperly formatted (check #R tag in file or the program arguments)");
    return;
  case OUT_OF_MEM:
      puts("Not enough memory");
      return;
  }
}
