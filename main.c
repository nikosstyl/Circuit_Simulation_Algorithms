#include <stdio.h>
#include "Parser/parser.h"


int main(int argc, char* argv[]) {
	FILE *input_file = NULL;
	Element *head = NULL;
	NodePair *head_node_pair = NULL;
	RetHelper ret = {0};
	SpiceAnalysis options={0};
	gsl_vector **x = NULL;

	ret.tolerance = 1e-3;

	if (argc != 2) {
		// no input file selected
		print_error(argv[0], 1, NULL);
	}

	// Open file
	input_file = fopen(argv[1], "rb");
	if (input_file == NULL) {
		print_error(argv[0], 2, NULL);
	}

	parser(input_file, &head, &head_node_pair,&ret, &options);

//	printf("List length: %d\n\n", get_list_length(head));

    // print_list(head);
    // print_pairs(head_node_pair);

//	printf("\nM1 elements: %d\nM2 elements: %d\n\n", ret.m1, ret.m2);

	create_matrix(head_node_pair, head, &ret, options, &x, argv[1]);


	fclose(input_file);
	fflush(stdout);
	free_mem(NULL, head, head_node_pair);

	return 0;
}
