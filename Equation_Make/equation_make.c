#include "parser.h"
#include <stdlib.h>
#include <stdio.h>

short int *create_matrix(NodePair *HashTable, Element *Element_list){

    Element *current = NULL;
	short int **A1=NULL, **A2 = NULL; // A[nodes_num][elements_num]
	int elements_number=0;
	int i=0;
	int hash_p=-1, hash_n=-1;
    unsigned long group1 = el_total_size - group2_size;

    if (!Element_list) {
		print_error("equation_make", 4, "Element list head empty");
	}

    if(amount_of_nodes-1 > 0){
        A1 = calloc(amount_of_nodes-1, sizeof(short int *));
        if(A1 == NULL){
            printf("Malloc failed. Ending.\n");
            return NULL;
        }
        A2 = calloc(amount_of_nodes-1, sizeof(short int *));
        if(A2 == NULL){
            printf("Malloc failed. Ending.\n");
            return NULL;
        }
        for (i=0;i<amount_of_nodes-1;i++){
            if(el_total_size-group2_size>0){
                A1[i] = calloc((el_total_size-group2_size), sizeof(short int));
                for(i =0; i < amount_of_nodes-1; i++){
                    if(A1[i] = NULL){
                        printf("Malloc failed. Ending.\n");
                        free(A1);
                        return NULL;
                    }
                }
            }
            else{
                free(A1);
            }    
        }
        for(i=0;i<amount_of_nodes-1;i++){
            if(group2_size>0){
                A2[i] = calloc((group2_size), sizeof(short int));
                if(*A2 = NULL){
                    printf("Malloc failed. Ending.\n");
                    free(A2);
                    return NULL;
                }
            }
            else{
                free(A2);
            }    
        }    
    }    

	// Fill A matrix for debug (reduced incidence matrix)
	int i=0;
	int j=0;
	for (current = Element_list;current->next != NULL;current=current->next) {

		// Differentiation using m1 and m2
//		if (current->type_of_element);

		hash_p = find_node_pair(HashTable, current->node_p);
		if (hash_p != 0) {
//			A[hash_p-1][i] = +1;
			if (!current->group_flag) {
					A1[hash_p-1][i] = +1;
			}
            else{
                A2[hash_p-1][j] = +1;
				}
			}
		}

		hash_n = find_node_pair(HashTable, current->node_n);
		if (hash_n != 0) {
			if (!current->group_flag) {
					A1[hash_p-1][i] = +1;
			}
            else{
                A2[hash_p-1][j] = +1;
			}
			
		}

	// Print A1 matrix
	for (i=0;i<amount_of_nodes;i++) {
		for (int j=0;j<el_total_size-group2_size;j++) {
			printf("%3d ", A1[i][j]);
		}
		printf("\n");
	}
	printf("\n");


	// Print A2 matrix
	for (i=0;i<amount_of_nodes;i++) {
		for (int j=0;j<group2_size;j++) {
			printf("%3d ", A2[i][j]);
		}
		printf("\n");
	}

	// Free Mem
	for (i=0;i<amount_of_nodes;i++) {
		free(A1[i]);
		free(A2[i]);
	}
	free(A1);
	free(A2);

    return 0;
}
