#include <stdio.h>
#include "Parser/parser.h"

int main(int argc, char* argv[]) {
    FILE *input_file = NULL;
    Element *head = NULL;
    NodePair *head_node_pair = NULL;

    if (argc != 2) {
        // no input file selected
        print_error("parser", 1, NULL);
    }

    // Open file
    input_file = fopen(argv[1], "rb");
    if (input_file == NULL) {
        print_error("parser", 2, NULL);
    }

    parser(input_file, &head, &head_node_pair);


    print_list(head);
    print_pairs(head_node_pair);

    fclose(input_file);
    fflush(stdout);
    free_mem(NULL, head, head_node_pair);

    return 0;
}