# Define the compiler and compiler flags
CC = gcc
CFLAGS = -Wall -I.

# Define the source files and object files
SRC = main.c Parser/parser.c Parser/equation_make.c
OBJ = $(SRC:.c=.o)

# Define the executable name
EXECUTABLE = my_program

# Define the default target (the one that gets built when you just run `make`)
all: $(EXECUTABLE)

# Build the executable
$(EXECUTABLE): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Build object files
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Run the program with an example input file
run: $(EXECUTABLE)
	./$(EXECUTABLE) Parser/test_netlist.cir

# Clean up the generated files
clean:
	rm -f $(EXECUTABLE) $(OBJ)

.PHONY: all clean run
