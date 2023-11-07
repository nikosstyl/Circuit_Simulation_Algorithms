#ifndef CIRCUIT_SIMULATION_ALGORITHMS_EQUATION_MAKE_H
#define CIRCUIT_SIMULATION_ALGORITHMS_EQUATION_MAKE_H

#include "../Parser/parser.h"

// Creates the equations needed for simulation
int equation_make(Element *head, NodePair *pair_head, int nodes_num);

// Returns the length of the Elements list
int get_list_length (Element *head);

#endif //CIRCUIT_SIMULATION_ALGORITHMS_EQUATION_MAKE_H
