#include "parser.h"
#include <stdlib.h>
#include <stdio.h>

int *create_matrix(NodePair *HashTable, Element *Element_list){

    Element *current = NULL;
	double **A=NULL, *b = NULL; // A[nodes_num][elements_num]
	int elements_number=0;
	int i=0;
	int hash_p=-1, hash_n=-1;

    if (!Element_list) {
		print_error("equation_make", 4, "Element list head empty");
	}

    b = calloc(amount_of_nodes-1+group2_size, sizeof(double));
    if(!b){
        printf("Something went wrong with stage 1 memory alloc. Exiting.\n");
        return -1;
    }

    A = calloc((amount_of_nodes-1)+group2_size, sizeof(double));
    if(!A){
        free(b);
        printf("Something went wrong with stage 1 memory alloc. Exiting.\n");
        return -1;
    }
    for(i=0; i < (amount_of_nodes-1+group2_size); i++){
        A[i] = calloc(el_total_size,sizeof(double));
        if(!A[i]){
            printf("Something went wrong with stage 2 memory alloc. Exiting.\n");
            for(int j=0; j < i; j++){
                free(A[j]);
            }
            free(A);
            free(b);
            return -1;
        }
    }
    for(i=0; i < amount_of_nodes-1+group2_size; i++){
        b[i] = 0;
        for(int j =0; j < el_total_size; j ++){
            A[i][j] = 0;
        }
    }

    current = Element_list;
    for(i = 0; i < el_total_size; i++){
        hash_p = find_node_pair(HashTable, current->node_p);
		hash_n = find_node_pair(HashTable, current->node_n);
        switch (Element_list->type_of_element)
        {
        case 'v':
            if(strcmp(current->node_p,"0")!=0){
                A[hash_p-1][amount_of_nodes-1+group2_size] = A[hash_p-1][amount_of_nodes-1+group2_size] + 1;
                A[amount_of_nodes-1+group2_size][hash_p-1] = A[amount_of_nodes-1+group2_size][hash_p-1]+1;
                b[hash_p-1] = b[hash_p-1]+current->value;
            }
            if(strcmp(current->node_n,"0")!=0){
                A[hash_n-1][amount_of_nodes-1+group2_size] = A[hash_n-1][amount_of_nodes-1+group2_size]-1;
                A[amount_of_nodes-1+group2_size][hash_n-1] = A[amount_of_nodes-1+group2_size][hash_n-1]-1;
                b[hash_n-1] = b[hash_n-1]-current->value;
            }
            break;
        case 'i':
            if(strcmp(current->node_p,"0")!=0){
                b[hash_p-1] = b[hash_p-1]+current->value;
            }
            if(strcmp(current->node_n,"0")!=0){
                b[hash_n-1] = b[hash_n-1]-current->value;
            }
            break;
        case 'r':
            if(strcmp(current->node_p,"0")!=0){
                A[hash_p-1][hash_p-1] = A[hash_p-1][hash_p-1] + (1/current->value);
                if(strcmp(current->node_n,"0")!=0){
                    A[hash_n-1][hash_p-1] = A[hash_n-1][hash_p-1]-(1/current->value);
                    A[hash_p-1][hash_n-1] = A[hash_p-1][hash_n-1] - (1/current->value);
                }
            }
            if(strcmp(current->node_n,"0")!=0){
                A[hash_n-1][hash_n-1] = A[hash_n-1][hash_n-1] + (1/current->value);
            }
            break;
        case 'l':
            if(strcmp(current->node_p,"0")!=0){
                b[hash_p-1] = b[hash_p-1]+0;
            }
            if(strcmp(current->node_n,"0")!=0){
                b[hash_n-1] = b[hash_n-1]-0;
            }
            break;
        case 'c':
            break;        
        default:
            break;
        }
        current = current->next;
    }

    printf("A table is: \n");
    for(i = 0; i < amount_of_nodes-1+group2_size; i++){
        for(int j=0; j < amount_of_nodes-1+group2_size; j++){
            printf("%.5lf ",A[i][j]);
        }
        printf("\n");
    }
    printf("b table is: \n");
    for(i = 0; i < amount_of_nodes-1+group2_size; i++){
        printf("%.5lf ",b[i]);
    }
    return 0;
}
