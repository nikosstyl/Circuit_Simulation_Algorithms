#include "equation_make.h"

int equation_make(Element *head, NodePair *pair_head, RetHelper helper) {
	Element *current = NULL;
	short int **A1=NULL, **A2=NULL; // A1[nodes_num][m1] A1[nodes_num][m2]
	int i=0, j=0;
	int hash_p=-1, hash_n=-1;

	if (!head) {
		print_error("equation_make", 4, "Element list head empty");
	}

	// Create A1 matrix
	A1 = calloc(helper.node_num, sizeof(short int *));
	if (!A1) {
		print_error("equation_make", 3, "A1 array error!");
	}
	for (i=0;i<helper.node_num;i++) {
		A1[i] = calloc((helper.m1), sizeof(short int));
		if (!A1[i]) {
			print_error("equation_make", 3, "Internal A1 array error!");
		}
	}

	// Create A2 matrix
	A2 = calloc(helper.node_num, sizeof(short int *));
	if (!A2) {
		print_error("equation_make", 3, "A2 array error!");
	}
	for (i=0;i<helper.node_num;i++) {
		A2[i] = calloc((helper.m2), sizeof(short int));
		if (!A2[i]) {
			print_error("equation_make", 3, "Internal A2 array error!");
		}
	}

	// Fill A matrix for debug (reduced incidence matrix)
	i=0;
	j=0;
	for (current = head;current->next != NULL;current=current->next) {

		// Differentiation using m1 and m2
//		if (current->type_of_element);

		hash_p = find_node_pair(pair_head, current->node_p);
		if (hash_p != 0) {
//			A[hash_p-1][i] = +1;
			switch (current->type_of_element) {
				case 'r': {}
				case 'c': {}
				case 'i': {
					A1[hash_p-1][i] = +1;
					break;
				}
				case 'l':{}
				case 'v':{
					A2[hash_p-1][j] = +1;
					break;
				}
			}
		}

		hash_n = find_node_pair(pair_head, current->node_n);
		if (hash_n != 0) {
			switch (current->type_of_element) {
				case 'r': {}
				case 'c': {}
				case 'i': {
					A1[hash_n-1][i++] = -1;
					break;
				}
				case 'l':{}
				case 'v':{
					A2[hash_n-1][j++] = -1;
					break;
				}
			}
		}
	}

	// Print A1 matrix
	for (i=0;i<helper.node_num;i++) {
		for (int j=0;j<helper.m1;j++) {
			printf("%3d ", A1[i][j]);
		}
		printf("\n");
	}
	printf("\n");


	// Print A2 matrix
	for (i=0;i<helper.node_num;i++) {
		for (int j=0;j<helper.m2;j++) {
			printf("%3d ", A2[i][j]);
		}
		printf("\n");
	}

	// Free Mem
	for (i=0;i<helper.node_num;i++) {
		free(A1[i]);
		free(A2[i]);
	}
	free(A1);
	free(A2);

	return -1;
}

int get_list_length (Element *head) {
	int i=0;
	for (Element *current=head;current!=NULL;current=current->next) {
		i++;
	}
	return(i-1);
}