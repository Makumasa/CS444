CC      = gcc
SOURCES = main.c rand.c mt19937ar.c
HEADERS = rand.h mt.h
FILES   = $(SOURCES) $(HEADERS)
OUTPUT  = assignment1

release: $(FILES)
	$(CC) -O2 -o $(OUTPUT) $(SOURCES)

debug: $(FILES)
	$(CC) -g -Wall -o $(OUTPUT) $(SOURCES)

clean:
	rm $(OUTPUT)

rebuild_release: clean release

rebuild_debug: clean debug

