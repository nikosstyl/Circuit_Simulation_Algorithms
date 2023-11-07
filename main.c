#include <stdio.h>
#include "Parser/parser.h"
#include "Equation_Make/equation_make.h"

int main(int argc, char* argv[]) {
    FILE *input_file = NULL;
    Element *head = NULL;
    NodePair *head_node_pair = NULL;
	RetHelper ret = {0};

    if (argc != 2) {
        // no input file selected
        print_error(argv[0], 1, NULL);
    }

    // Open file
    input_file = fopen(argv[1], "rb");
    if (input_file == NULL) {
        print_error(argv[0], 2, NULL);
    }

    ret = parser(input_file, &head, &head_node_pair);

	printf("List length: %d\n\n", get_list_length(head));

    print_list(head);
    print_pairs(head_node_pair);

	printf("\nM1 elements: %d\nM2 elements: %d\n\n", ret.m1, ret.m2);

	equation_make(head, head_node_pair, ret);

    fclose(input_file);
    fflush(stdout);
    free_mem(NULL, head, head_node_pair);

    return 0;
}