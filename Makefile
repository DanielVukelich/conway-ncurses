kCC=gcc

CFLAGS=-std=c11 -Werror -Wall

DEBUGFLAGS=-DDEBUG -g -fsanitize=address -fsanitize=undefined
DEBUGLIBS=-lubsan

RELEASEFLAGS=-O3

LDLIBS=-lncurses
SRCDIR=src
INC=$(SRCDIR)/

TESTOBJ=$(SRCDIR)/test.o
BINOBJ=$(SRCDIR)/gameoflife.o

OBJS=$(SRCDIR)/gamefield.o $(SRCDIR)/bit_accessor.o
BINARY=lifegame
TEST=testsuite
BINDIR=build/

.PHONY: clean all debug release tests

all: debug

tests: CFLAGS += $(DEBUGFLAGS)
tests: LDLIBS += $(DEBUGLIBS)
tests: $(TEST)
	./$(BINDIR)$(TEST)

debug: CFLAGS += $(DEBUGFLAGS)
debug: LBLIBS += $(DEBUGLIBS)
debug: $(BINARY)

release: CFLAGS += $(RELEASEFLAGS)
release: $(BINARY)

$(BINARY): $(OBJS) $(BINOBJ)
	$(CC) -o $(BINDIR)$@ $(CFLAGS) $(BINOBJ) $(OBJS) $(LDLIBS)

$(TEST): $(OBJS) $(TESTOBJ)
	$(CC) -o $(BINDIR)$@ $(CFLAGS) $(TESTOBJ) $(OBJS) $(LDLIBS)

%.o: %.c %.h
	$(CC) -c $< -o $@ $(CFLAGS)

clean: OBJS += $(TESTOBJ) $(BINOBJ)
clean:
	rm -f $(BINDIR)$(BINARY) $(TEST) $(OBJS) asan.log*

$(shell mkdir -p $(BINDIR))
