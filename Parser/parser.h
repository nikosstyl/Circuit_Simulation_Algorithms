#define NUM_OF_ELEMENT_DATA 5
#define MAX_CHAR_NUM 100

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>

static const char DC_ANALYSIS[] = ".op";
static const char SKIP_NEWLINE[] = "%*[^\n]\n";


// Struct of an element
// Data for the simulation
struct sim_element {
	char type_of_element;
	char *name;
	char* node_p;
	char* node_n;
	double value;
	int group_flag;
	
	//group_flag determines the group of the element. If set to 0 then KCL is needed to describe it, if set to 1, KVL is needed. 
	// Hashed nodes
	// unsigned long node_p_hash;
	// unsigned long node_n_hash; 

	// Linked list variables
	struct sim_element *next;
	struct sim_element *prev;
};
typedef struct sim_element Element;

// Linked list that stores the hash num and string and the size of the hash table
struct node_pair {
	unsigned long hash_node_num;
	char * node_str;
	struct node_pair *next;
	
};
typedef struct node_pair NodePair;

// Struct that holds information for the analysis of the simulation
struct analysis_type {
	char* type;
	int start_voltage;
	int stop_voltage;
	unsigned long start_frequency;
	unsigned long stop_frequency;
	double sim_time;
};
typedef struct analysis_type AnalysisType;

struct ret_helper {
	unsigned long int group2_size;
	unsigned long int el_total_size;
	unsigned long int amount_of_nodes;
};
typedef struct ret_helper RetHelper;

// Main parser function. It calls every other function here. Returns the amount of elements that belong to group 2.
void parser(FILE *input_file, Element **head, NodePair **head_node_pair, RetHelper *ret);

// Get analysis type from line string
void get_analysis_type(char* line, AnalysisType **type_struct);

// Prints according error in stderr and terminates the program.
void print_error (char* program_name, int error_code, char* comment);

// Free up any memory used.
void free_mem (char** lines, Element *head, NodePair *head_pair);

// Takes a line from the file and splits it in a 2D matrix, so its easier to split it afterwards.
// Returns 1 on success and 0 on error.
int remove_spaces(char *line, char **line_array);

// Prints the linked list with the elements.
void print_list(Element *head);

// tolower() but for string.
void strToLower(char *str);

// Hash function. Basicly, it's djb2 hash function.
unsigned long hash(char * str);

// Adds node hash number pair and node string to a db that contains every pair.
// Returns 0 on error and 1 on success
int add_node_pair(NodePair **head, unsigned long hash_num, char* node_str);

// Finds a pair of node hash number and node string in a db (linked list).
// Returns -1 if hash num not found, else it returns the hash num.
int find_node_pair(NodePair *head, char* node_str);

// Prints every pair in the db (linked list).
void print_pairs(NodePair *head);

int create_matrix(NodePair *HashTable, Element *Element_list, RetHelper *ret);
