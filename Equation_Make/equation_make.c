#include "equation_make.h"

int equation_make(Element *head, NodePair *pair_head, RetHelper helper) {
	Element *current = NULL;
	short int **A1=NULL, **A2=NULL; // A1[nodes_num][m1] A2[nodes_num][m2]
	double **G_diag=NULL, **C_diag=NULL, **L_diag=NULL; // G[m1][m1] C[m1][m1] L[m2][m2]
	double *s1=NULL, *s2=NULL; // s1[m1] s2[m2]
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

	// Create G matrix
	G_diag = calloc(helper.m1, sizeof(double *));
	if (!G_diag) {
		print_error("equation_make", 3, "G array error!");
	}
	for (i=0;i<helper.m1;i++) {
		G_diag[i] = calloc((helper.m1), sizeof(double));
		if (!G_diag[i]) {
			print_error("equation_make", 3, "Internal G array error!");
		}
	}

	// Create C matrix
	C_diag = calloc(helper.m1, sizeof(double *));
	if (!C_diag) {
		print_error("equation_make", 3, "C array error!");
	}
	for (i=0;i<helper.m1;i++) {
		C_diag[i] = calloc((helper.m1), sizeof(double));
		if (!C_diag[i]) {
			print_error("equation_make", 3, "Internal C array error!");
		}
	}

	// Create L matrix
	L_diag = calloc(helper.m2, sizeof(double *));
	if (!L_diag) {
		print_error("equation_make", 3, "L array error!");
	}
	for (i=0;i<helper.m2;i++) {
		L_diag[i] = calloc((helper.m2), sizeof(double));
		if (!L_diag[i]) {
			print_error("equation_make", 3, "Internal L array error!");
		}
	}

	// Create s1 matrix
	s1 = calloc(helper.m1, sizeof(double));
	if (!s1) {
		print_error("equation_make", 3, "S1 array error!");
	}

	// Create s2 matrix
	s2 = calloc(helper.m2, sizeof(double));
	if (!s2) {
		print_error("equation_make", 3, "S2 array error!");
	}

	// Fill A matrix for debug (reduced incidence matrix)
	i=0;
	j=0;
	for (current = head;current->next != NULL;current=current->next) {
		// Differentiation using m1 and m2

		hash_p = find_node_pair(pair_head, current->node_p);
		if (hash_p != 0) {
			switch (current->type_of_element) {
				case 'r': {
					if (current->type_of_element == 'r') {
						G_diag[i][i] = current->value;
					}
				}
				case 'c': {
					if (current->type_of_element == 'c') {
						 C_diag[i][i] = current->value;
					}
				}
				case 'i': {
					if (current->type_of_element == 'i') {
						 s1[i] = current->value;
					}
					A1[hash_p-1][i] = +1;
					break;
				}
				case 'l':{
					if (current->type_of_element == 'l') {
						 L_diag[j][j] = current->value;
					}
				}
				case 'v':{
					if (current->type_of_element == 'v') {
						 s2[j] = current->value;
					}
					A2[hash_p-1][j] = +1;
					break;
				}
			}
		}

		hash_n = find_node_pair(pair_head, current->node_n);
		if (hash_n != 0) {
			switch (current->type_of_element) {
				case 'r': {
					if (current->type_of_element == 'r') {
						G_diag[i][i] = current->value;
					}
				}
				case 'c': {
					if (current->type_of_element == 'c') {
						C_diag[i][i] = current->value;
					}
				}
				case 'i': {
					if (current->type_of_element == 'i') {
						s1[i] = current->value;
					}
					A1[hash_n-1][i++] = -1;
					break;
				}
				case 'l':{
					if (current->type_of_element == 'l') {
						L_diag[j][j] = current->value;
					}
				}
				case 'v':{
					if (current->type_of_element == 'v') {
						s2[j] = current->value;
					}
					A2[hash_n-1][j++] = -1;
					break;
				}
			}
		}
	}


	// Print A1 matrix
	printf("\nA1:\n");
	for (i=0;i<helper.node_num;i++) {
		for (j=0;j<helper.m1;j++) {
			printf("%3d ", A1[i][j]);
		}
		printf("\n");
	}
	printf("\n");


	// Print A2 matrix
	printf("\nA2:\n");
	for (i=0;i<helper.node_num;i++) {
		for (j=0;j<helper.m2;j++) {
			printf("%3d ", A2[i][j]);
		}
		printf("\n");
	}

	// Print G matrix
	printf("\nG_diag:\n");
	for (i=0;i<helper.m1;i++) {
		for (j=0;j<helper.m1;j++) {
			if(G_diag[i][j] != 0) {
				printf("\033[32m%10.1lf\033[0m ", G_diag[i][j]);
			}
			else {
				printf("\033[31m%10.1lf\033[0m ", G_diag[i][j]);
			}
		}
		printf("\n");
	}
	printf("\n");

	// Print C matrix
	printf("\nC_diag:\n");
	for (i=0;i<helper.m1;i++) {
		for (j=0;j<helper.m1;j++) {
			if(C_diag[i][j] != 0) {
				printf("\033[32m%12.4lf\033[0m ", C_diag[i][j]);
			}
			else {
				printf("\033[31m%12.4lf\033[0m ", C_diag[i][j]);
			}
		}
		printf("\n");
	}
	printf("\n");

	// Print L matrix
	printf("\nL_diag:\n");
	for (i=0;i<helper.m2;i++) {
		for (j=0;j<helper.m2;j++) {
			if(L_diag[i][j] != 0) {
				printf("\033[32m%12.4lf\033[0m ", L_diag[i][j]);
			}
			else {
				printf("\033[31m%12.4lf\033[0m ", L_diag[i][j]);
			}
		}
		printf("\n");
	}
	printf("\n");

	// Print S1 matrix
	printf("\nS1:\n");
	for(i=0;i<helper.m1;i++) {
		if(s1[i] != 0) {
			printf("\033[32m%3.1lf\033[0m\n", s1[i]);
		}
		else {
			printf("\033[31m%3.1lf\033[0m\n", s1[i]);
		}
	}
	printf("\n");

	// Print S2 matrix
	printf("\nS2:\n");
	for(i=0;i<helper.m2;i++) {
		if(s2[i] != 0) {
			printf("\033[32m%3.1lf\033[0m\n", s2[i]);
		}
		else {
			printf("\033[31m%3.1lf\033[0m\n", s2[i]);
		}
	}
	printf("\n");

	// Free Mem
	for (i=0;i<helper.node_num;i++) {
		free(A1[i]);
		free(A2[i]);
	}
	for (i=0; i<helper.m1; i++) {
		free(G_diag[i]);
		free(C_diag[i]);
	}
	for(i=0;i<helper.m2;i++) {
		free(L_diag[i]);
	}
	free(A1);
	free(A2);
	free(G_diag);
	free(C_diag);
	free(L_diag);
	free(s1);
	free(s2);

	return -1;
}

int get_list_length (Element *head) {
	int i=0;
	for (Element *current=head;current!=NULL;current=current->next) {
		i++;
	}
	return(i-1);
}

