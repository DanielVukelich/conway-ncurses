# conway-ncurses
An Ncurses implementation of [Conway's Game of Life](https://en.wikipedia.org/wiki/Conway's_Game_of_Life) and [other cellular automaton rules](http://conwaylife.com/wiki/Rules)

## Building
Build a debug version with `make`, and a version without debug symbols by running `make release`.  The program will be put in the `build/` directory.  Make sure that you have `ncurses-dev` or your distro's equivalent package installed.

## Running
Running the program without any arguments will launch a random game with Conway's rules.  To exit, press 'q'.  To pause, press space.  When you are paused, you can press 's' to step forward one generation at a time.

The program supports the following optional command line arguments:
* `--file <path>` or `-f <path>`:  Loads a Life 1.05 file describing a pattern and optional ruleset.
* `--seed <num>` or `-s <num>`:  When generating a random game, 1/num of every cell will be seeded 'alive'.  The higher num, the more cells will start as 'dead'.  The seed is not allowed to be a value less than 1
* `--rule <string>` or `-r <string>`:  Tells the game what rules determine how cells live and die.  If a file input is specified and that file uses tags that tell what ruleset to use, the file's rules will take precedence.  Specify rules in the standard Alive/Born format.  Eg:  Conway's original rules are that a living cell will continue to live if it has 2 or 3 living neighbours, and a dead cell will be reborn if it has exactly 3 living neighbours.  This ruleset would be passed as `--rule 23/3`.  If you wanted cells to stay alive if they have an even number of living neighbours and dead cells to be born if they have exactly 1 neighbour, you would use the ruleset `--rule 2468/1`.  Note that since a cell cannot have more than 8 neighbours, the number 9 is not allowed in the rule string.
* `--widescreen` or `-w`:  Because the characters in a terminal are taller than they are wide, one cell in the game is represented by 2 screen characters.  This is so things look more uniformly square.  If you would like to have extra horizontal resolution, passing `--widescreen` will use one character to draw one cell.
* `--infinite` or `-i`:  If this flag is enabled, cells will "wrap" around the borders.  For example, a glider flying toward into the right border will reappear on the left border (still flying right).
* `--pause` or `-p`:  If this flag is enabled, the game will begin paused (press space to unpause).  Useful if you want to examine a pattern at the beginning.

## Life 1.05
This program can load files in the Life 1.05 file format.  Information about the Life 1.05 file format [can be found here.](http://conwaylife.com/wiki/Life_1.05)

One minor thing to note is that while the Life 1.05 format calls for DOS style line endings, the program currently only supports UNIX line endings.
