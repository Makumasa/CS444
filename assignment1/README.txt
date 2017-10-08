"make release" will compile the program (assignment1) and store it in
the "build" subdirectory. 

"make debug" will compile the program with debug symbols and all warnings.

The program, assignment1, takes either 0 or 2 arguments. If no arguments are
provided, the program will create 5 producers and 3 consumers. If 2 (or more)
arguments are provided, the first argument will be used as the number of
producers and the second argument will be used as the number of consumers.

To end the program, send it an interrupt signal (Ctrl+C).