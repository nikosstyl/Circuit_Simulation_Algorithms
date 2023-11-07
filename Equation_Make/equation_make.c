#include "equation_make.h"

int equation_make(Element *head, NodePair *pair_head, RetHelper helper) {
	Element *current = NULL;
	short int **A=NULL; // A[nodes_num][elements_num]
	int elements_number=0;
	int i=0;
	int hash_p=-1, hash_n=-1;

	// Create A matrix
	A = calloc(helper.node_num, sizeof(short int *));
	for (i=0;i<helper.node_num;i++) {
		A[i] = calloc((helper.m1+helper.m2), sizeof(short int));
	}

	if (!head) {
		print_error("equation_make", 4, "Element list head empty");
	}

	// Fill A matrix for debug (reduced incidence matrix)
	for (current = head,i=0;current->next != NULL;current=current->next,i++) {

		// Differentiation using m1 and m2
//		if (current->type_of_element);

		hash_p = find_node_pair(pair_head, current->node_p);
		if (hash_p != 0) {
			A[hash_p-1][i] = +1;
		}

		hash_n = find_node_pair(pair_head, current->node_n);
		if (hash_n != 0) {
			A[hash_n-1][i] = -1;
		}
	}

	// Print A matrix
	for (i=0;i<helper.node_num;i++) {
		for (int j=0;j<helper.m1+helper.m2;j++) {
			printf("%3d ", A[i][j]);
		}
		printf("\n");
	}

	// Free Mem
	for (i=0;i<helper.node_num;i++) {
		free(A[i]);
	}
	free(A);

	return -1;
}

int get_list_length (Element *head) {
	int i=0;
	for (Element *current=head;current!=NULL;current=current->next) {
		i++;
	}
	return(i-1);
}