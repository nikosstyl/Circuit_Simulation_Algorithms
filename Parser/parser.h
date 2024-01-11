#ifndef PARSER_H
#define PARSER_H
#define NUM_OF_ELEMENT_DATA 5
#define MAX_CHAR_NUM 100
#define MAX_LINE_BUFF_LEN 1000
#define STARTING_ARR_NUM 5
#define EPS 1e-14
#define MAX_ITERATIONS 1000
#define ERR_NO_INPUT_FILE 1
#define ERR_NO_FILE_READ 2
#define ERR_NO_MEM 3
#define ERR_GENERAL 4

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_blas.h>
#include <suitesparse/cs.h>

static const char SPICE_END[] = ".end";
static const char DC_ANALYSIS[] = ".op";
static const char OPTIONS[] = ".options";
static const char DC_SWEEP[] = ".dc";
static const char PRINT[] = ".print";
static const char PLOT[] = ".plot";

static const char CHOLESKY_OPTION[] = "spd";
static const char USE_ITERATIONS_OPTION[] = "iter";
static const char GET_TOLERANCE[] = "itol=";
static const char SPARSE_OPTION[] = "sparse";

static const char SKIP_NEWLINE[] = "%*[^\n]\n";
static const char RED[] = "\x1b[31m";
static const char GREEN[] = "\x1b[32m";
static const char YELLOW[] = "\x1b[33m";
static const char BLUE[] = "\x1b[34m";
static const char RESET[] = "\x1b[0m";


// Struct of an element
// Data for the simulation
struct sim_element {
	char type_of_element;
	char *name;
	char* node_p;
	char* node_n;
	double value;
	int group_flag;
	int position_in_vector_B;
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

// Struct that holds information for all spice options
struct spice_analysis {
	// Element *head;
    bool DC_OP;
    struct dc_sweep_opts* DC_SWEEP;
    struct plot_opts* PLOT;
};
typedef struct spice_analysis SpiceAnalysis;

// Struct for holding information about DC Sweep
struct dc_sweep_opts {
	char variable_type;
    char *variable_name;
	bool is_voltage; // true for voltage, false for current
    double start_val;
    double end_val;
    double increment;
};

// Struct for holding information about Plots
struct plot_opts {
    int str_num; // Total number of strings aka V/I(<nodes>)
    char **elements_to_print; // elements_to_print[str_num]
};

struct ret_helper {
	unsigned long int group2_size;
	unsigned long int el_total_size;
	unsigned long int amount_of_nodes;
	short int direct_chol_flag;
	short int use_iterations;
	short int use_iterations_cg;
	double tolerance;
	int non_zero_elements;
	bool sparse;
};
typedef struct ret_helper RetHelper;

// Main parser function. It calls every other function here. Returns the amount of elements that belong to group 2.
void parser(FILE *input_file, Element **head, NodePair **head_node_pair, RetHelper *ret, SpiceAnalysis *options);

// Prints according error in stderr and terminates the program.
void print_error (char* program_name, int error_code, const char* comment);

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

int create_matrix(NodePair *HashTable, Element *Element_list, RetHelper *ret, SpiceAnalysis options, gsl_vector ***x, char *filename);

int find_b_pos (char *element_name, char type, Element *head, int *out, NodePair *HashTable);

void print_equation_system (RetHelper helper, gsl_matrix *A, gsl_vector *B);

void cg_solve(gsl_matrix *A, gsl_vector *b, gsl_vector **x, double itol, int n);

void bicg_solve(gsl_matrix *A, gsl_vector *b, gsl_vector **x, double itol, int n);

void sparse_bi_cg_iter (const cs *A, const gsl_vector *b, gsl_vector **x, double itol);

void sparse_cg_iter (const cs *A, const gsl_vector *b, gsl_vector **x, double itol);

void sparse_direct_equation_solve(cs *A, gsl_vector *B, gsl_vector ***x, SpiceAnalysis options, Element *head, RetHelper helper, NodePair *pair_head);

void sparse_iterative_equation_solve (const cs *A, gsl_vector *B, gsl_vector ***x, SpiceAnalysis options, Element *head, RetHelper helper, NodePair *pair_head);

void plot(char *analysis_name, gsl_vector **x, NodePair *pair_head, RetHelper helper, SpiceAnalysis options);
#endif