CC          = gcc
LINK_FLAGS  = 
CFLAGS      = $(LINK_FLAGS) -pthread -std=c99 -O2 -DNDEBUG
DEBUG_FLAGS = $(LINK_FLAGS) -pthread -std=c99 -Og -g -Wall -DDEBUG
PROGRAMS    = problem1
HEADERS     = mt19937-64.h

.PHONY: debug all clean rebuild rebuild_debug

all: $(PROGRAMS)

debug: CFLAGS := $(DEBUG_FLAGS)
debug: $(PROGRAMS)

problem1: problem1.c mt19937-64.c $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $(filter %.c,$^)

clean:
	rm -rf $(PROGRAMS) *.o *.s

rebuild: clean all

rebuild_debug: clean debug
