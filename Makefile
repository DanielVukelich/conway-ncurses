CC=gcc
CFLAGS=-std=c11 -Werror -Wall
LDLIBS=-lncurses
SRCDIR=src/
INC=$(SRCDIR)
OBJS=$(SRCDIR)gameoflife.o $(SRCDIR)gamefield.o
BINARY=lifegame
BINDIR=build/

.PHONY: clean all debug release

all: debug

debug: CFLAGS += -DDEBUG -g
debug: $(BINARY)

release: CFLAGS += -O3
release: $(BINARY)

$(BINARY): $(OBJS)
	$(CC) -o $@ $(CFLAGS) $(OBJS) $(LDLIBS)
	mv $(BINARY) $(BINDIR)$(BINARY)

%.o: %.c %.h
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -f $(BINDIR)$(BINARY) $(OBJS)
