#include "parser.h"
#include <stdlib.h>
#include <stdio.h>

int create_matrix(NodePair *HashTable, Element *Element_list, RetHelper *ret){

    Element *current = NULL;
	double **A=NULL, *b = NULL; // A[nodes_num][elements_num]
    unsigned long m2counter = 0;
	int elements_number=0;
	int i=0;
	int hash_p=-1, hash_n=-1;

    if (!Element_list) {
		print_error("equation_make", 4, "Element list head empty");
	}

    b = calloc((ret->amount_of_nodes+ret->group2_size), sizeof(double));
    if(!b){
        printf("Something went wrong with stage 1 memory alloc. Exiting.\n");
        return -1;
    }

    A = calloc((ret->amount_of_nodes+ret->group2_size), sizeof(double));
    if(!A){
        free(b);
        printf("Something went wrong with stage 1 memory alloc. Exiting.\n");
        return -1;
    }
    for(i=0; i < ((ret->amount_of_nodes+ret->group2_size)); i++){
        A[i] = calloc((ret->amount_of_nodes+ret->group2_size),sizeof(double));
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
    for(i=0; i < (ret->amount_of_nodes+ret->group2_size); i++){
        b[i] = 0;
        for(int j =0; j < (ret->amount_of_nodes+ret->group2_size); j ++){
            A[i][j] = 0;
        }
    }
    
    current = Element_list;
    for(current=Element_list;current->next!=NULL; current=current->next){
        hash_p = (find_node_pair(HashTable, current->node_p));
		hash_n = (find_node_pair(HashTable, current->node_n));
        
        switch (current->type_of_element)
        {
        case 'v':{
            m2counter++;
            if(hash_p!=0){
                
                A[hash_p-1][ret->amount_of_nodes+m2counter-1] = A[hash_p-1][ret->amount_of_nodes+m2counter-1] + 1.0;
                A[ret->amount_of_nodes+m2counter-1][hash_p-1] = A[ret->amount_of_nodes+m2counter-1][hash_p-1]+1.0;
                printf("Done correctly\n");
                b[hash_p-1] = b[hash_p-1]+current->value;
            }
            if(hash_n!=0){
                A[hash_n-1][ret->amount_of_nodes+m2counter-1] = A[hash_n-1][ret->amount_of_nodes+m2counter-1]-1.0;
                A[ret->amount_of_nodes+m2counter-1][hash_n-1] = A[ret->amount_of_nodes+m2counter-1][hash_n-1]-1.0;
                b[hash_n-1] = b[hash_n-1]-current->value;
            }
            break;
        }    
        case 'i':{
            if(hash_p!=0){
                b[hash_p-1] = b[hash_p-1]-current->value;
            }
            if(hash_n!=0){
                b[hash_n-1] = b[hash_n-1]+current->value;
            }
            break;
        }    
        case 'r':{
            if(hash_p!=0){
                A[hash_p-1][hash_p-1] = A[hash_p-1][hash_p-1] + (1/current->value);
                if(hash_n!=0){
                    A[hash_n-1][hash_p-1] = A[hash_n-1][hash_p-1]-(1/current->value);
                    A[hash_p-1][hash_n-1] = A[hash_p-1][hash_n-1] - (1/current->value);
                }
            }
            if(hash_n!=0){
                A[hash_n-1][hash_n-1] = A[hash_n-1][hash_n-1] + (1/current->value);
            }
            break;
        }    
        case 'l':{
            m2counter++;
            if(hash_p!=0){
                A[hash_p-1][ret->amount_of_nodes+m2counter-1] = A[hash_p-1][ret->amount_of_nodes+m2counter-1] + 1.0;
                A[ret->amount_of_nodes+m2counter-1][hash_p-1] = A[ret->amount_of_nodes+m2counter-1][hash_p-1]+1.0;
                b[hash_p-1] = b[hash_p-1]+0;
            }
            if(hash_n!=0){
                A[hash_n-1][ret->amount_of_nodes+m2counter-1] = A[hash_n-1][ret->amount_of_nodes+m2counter-1]-1.0;
                A[ret->amount_of_nodes+m2counter-1][hash_n-1] = A[ret->amount_of_nodes+m2counter-1][hash_n-1]-1.0;
                b[hash_n-1] = b[hash_n-1]-0;
            }
            break;
        }    
        case 'c':
            break;        
        default:
            break;
        }
        //current = current->next;
    }

    printf("A table is: \n");
    for(i = 0; i < ret->amount_of_nodes+ret->group2_size; i++){
        for(int j=0; j < ret->amount_of_nodes+ret->group2_size; j++){
            printf("%.5lf ",A[i][j]);
        }
        printf("\n");
    }
    printf("b table is: \n");
    for(i = 0; i < ret->amount_of_nodes+ret->group2_size; i++){
        printf("%.5lf ",b[i]);
    }
    return 0;
}
