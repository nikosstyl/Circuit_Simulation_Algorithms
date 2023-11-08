#include "equation_make.h"

int equation_make(Element *head, NodePair *pair_head, RetHelper helper) {
	Element *current = NULL;
	double **A=NULL, *B=NULL; // A[(nodes)+m2][(nodes)+m2] B[(nodes)+m2]
	int i=0, j=0;
	int hash_p=-1, hash_n=-1;

	if (!head) {
		print_error("equation_make", 4, "Element list head empty");
	}

	// Create the matrices needed for the simulation
	A = calloc(helper.node_num+helper.m2, sizeof(double *));
	if (!A) {
		print_error("equation_make", 3, "Matrix A couldn't be created");
	}
	for (i=0;i<helper.node_num+helper.m2;i++) {
		A[i] = calloc(helper.node_num+helper.m2, sizeof(double));
		if (!A[i]) {
			print_error("equation_make", 3, "Matrix A inside coulnd't be created");
		}
	}
	B = calloc(helper.node_num+helper.m2, sizeof(double));
	if (!B) {
		print_error("equation_make", 3, "Matrix B couldn't be created");
	}

	// Fill A matrix
	i=0;
	j=0;
	for (current = head;current->next != NULL;current=current->next) {
		// Differentiation using m1 and m2

		hash_p = find_node_pair(pair_head, current->node_p);
		hash_n = find_node_pair(pair_head, current->node_n);

		switch (current->type_of_element) {
			case 'r': {
				if ((hash_p !=0) && (hash_n != 0)) {
					A[hash_p-1][hash_p-1] += (1 / current->value);
					A[hash_p-1][hash_n-1] += -(1 / current->value);
					A[hash_n-1][hash_p-1] += -(1 / current->value);
					A[hash_n-1][hash_n-1] += (1 / current->value);
				}
				else if (hash_n == 0) {
					A[hash_p-1][hash_p-1] += (1 / current->value);
				}
				else {
					A[hash_n-1][hash_n-1] += (1 / current->value);
				}
				break;
			}
			case 'i': {
				if ((hash_p != 0) && (hash_n != 0)) {
					B[hash_p-1] = -(current->value);
					B[hash_n-1] = current->value;
				}
				else if (hash_n == 0) {
					B[hash_p-1] = -(current->value);
				}
				else {
					B[hash_n-1] = current->value;
				}
				break;
			}
			case 'l': {
				// For inductors, treat them as a 0-volt voltage source
				// in DC analysis (often called and .op)
			}
			case 'v': {
				if ((hash_p != 0) && (hash_n != 0)) {
					A[helper.node_num + i][hash_p-1] = +1;
					A[helper.node_num + i][hash_n-1] = -1;
					A[hash_p-1][helper.node_num + i] = +1;
					A[hash_n-1][helper.node_num + i] = -1;
				}
				else if (hash_n ==0) {
					A[helper.node_num + i][hash_p-1] = +1;
					A[hash_p-1][helper.node_num + i] = +1;
				}
				else {
					A[helper.node_num + i][hash_n-1] = -1;
					A[helper.node_num + i][hash_n-1] = -1;
				}
				B[helper.node_num+i] = current->type_of_element=='v' ? current->value : 0;
				i++;
				break;
			}
			case 'c': {
				// Skip for DC analysis
				break;
			}
		}
	}

	printf("\nSysthma eksiswsewn:\n");
	for (i=0;i<helper.node_num+helper.m2;i++) {
		for (j=0;j<helper.node_num+helper.m2;j++) {
			if (i<helper.node_num && j<helper.node_num) {
				printf("%s%5.2lf ", RED, A[i][j]);
			}
			else if (i>=helper.node_num && j<helper.node_num) {
				printf("%s%5.2lf ", BLUE, A[i][j]);
			}
			else if (i<helper.node_num && j>=helper.node_num) {
				printf("%s%5.2lf ", GREEN, A[i][j]);
			}
			else{
				printf("%s%5.2lf ", YELLOW, A[i][j]);
			}
		}
		printf(" %s%5.2lf\n", RESET, B[i]);
	}
	printf("\n");

	// Free Mem
	for (i=0;i<helper.node_num+helper.m2;i++) {
		free(A[i]);
	}
	free(A);
	free(B);

	return -1;
}

int get_list_length (Element *head) {
	int i=0;
	for (Element *current=head;current!=NULL;current=current->next) {
		i++;
	}
	return(i-1);
}
