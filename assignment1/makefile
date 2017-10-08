CC      = gcc
CFLAGS  = -std=c99 -pthread
SOURCES = main.c rand.c mt19937ar.c eventqueue.c
HEADERS = rand.h mt.h eventqueue.h
FILES   = $(SOURCES) $(HEADERS)
OUTDIR  = ./build
OUTPUT  = $(OUTDIR)/assignment1

create_outdir:
	mkdir -p $(OUTDIR)

release: create_outdir $(FILES)
	$(CC) $(CFLAGS) -O2 -o $(OUTPUT) $(SOURCES)

debug: create_outdir $(FILES)
	$(CC) $(CFLAGS) -g -Wall -o $(OUTPUT) $(SOURCES)

clean: create_outdir
	rm -rf $(OUTPUT)

rebuild_release: clean release

rebuild_debug: clean debug
