JVERSION = 1.8
JC       = javac
JAR      = jar
OUTDIR   = out/
SRCDIR   = src/
SOURCES  = Fork.java Main.java Philosopher.java
OUTPUT   = Concurrency.jar

.PHONY: all clean compile create_outdir

all: $(OUTPUT)

run: all
	java -jar $(OUTPUT)

$(OUTPUT): compile
	$(JAR) cefv Main $(OUTPUT) -C $(OUTDIR) .
	
compile: create_outdir
	$(JC) -source $(JVERSION) -target $(JVERSION) -d $(OUTDIR) $(addprefix $(SRCDIR),$(SOURCES))
	
create_outdir:
	mkdir -p $(OUTDIR)
	
clean:
	rm -rf $(OUTDIR)* $(OUTPUT)
