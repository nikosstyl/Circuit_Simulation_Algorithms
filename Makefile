# Define the compiler and compiler flags
CC = gcc
CFLAGS = -Wall -g -I.
LIBFLAGS = -lm -lgsl -lgslcblas

# Define the source files and object files
SRC = main.c Parser/parser.c Parser/equation_make.c
OBJ = $(SRC:.c=.o)

# Define the executable name
EXECUTABLE = Circuit_Simulation

# Define the default target (the one that gets built when you just run `make`)
all: $(EXECUTABLE)

# Build the executable
$(EXECUTABLE): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBFLAGS)

# Build object files
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $< $(LIBFLAGS)

# Run the program with an example input file
run: $(EXECUTABLE)
	./$(EXECUTABLE) Parser/test_netlist.cir

# Clean up the generated files
clean:
	rm -f $(EXECUTABLE) $(OBJ) *.out

.PHONY: all clean run
