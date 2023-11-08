#ifndef CIRCUIT_SIMULATION_ALGORITHMS_EQUATION_MAKE_H
#define CIRCUIT_SIMULATION_ALGORITHMS_EQUATION_MAKE_H

#include "../Parser/parser.h"

static const char RED[] = "\x1b[31m";
static const char GREEN[] = "\x1b[32m";
static const char YELLOW[] = "\x1b[33m";
static const char BLUE[] = "\x1b[34m";
static const char RESET[] = "\x1b[0m";

// Creates the equations needed for simulation
int equation_make(Element *head, NodePair *pair_head, RetHelper helper);

// Returns the length of the Elements list
int get_list_length (Element *head);

#endif //CIRCUIT_SIMULATION_ALGORITHMS_EQUATION_MAKE_H
