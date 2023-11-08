#ifndef CIRCUIT_SIMULATION_ALGORITHMS_EQUATION_MAKE_H
#define CIRCUIT_SIMULATION_ALGORITHMS_EQUATION_MAKE_H

#define ORFEAS 1

#include "../Parser/parser.h"

// Creates the equations needed for simulation
int equation_make(Element *head, NodePair *pair_head, RetHelper helper);

// Returns the length of the Elements list
int get_list_length (Element *head);

#ifdef ORFEAS
int get_component_position (Element *head, Element *target);
#endif

#endif //CIRCUIT_SIMULATION_ALGORITHMS_EQUATION_MAKE_H
