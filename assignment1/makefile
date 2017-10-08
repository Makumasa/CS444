CC      = gcc
CFLAGS  = -std=c99 -pthread
SOURCES = main.c rand.c mt19937ar.c eventqueue.c
HEADERS = rand.h mt.h eventqueue.h
FILES   = $(SOURCES) $(HEADERS)
OUTPUT  = ./build/assignment1

release: $(FILES)
	$(CC) $(CFLAGS) -O2 -o $(OUTPUT) $(SOURCES)

debug: $(FILES)
	$(CC) $(CFLAGS) -g -Wall -o $(OUTPUT) $(SOURCES)

clean:
	rm $(OUTPUT)

rebuild_release: clean release

rebuild_debug: clean debug

