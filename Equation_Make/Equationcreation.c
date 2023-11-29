#include "parser.h"
#include <stdlib.h>
#include <stdio.h>
#include <gsl/gsl_linalg.h>

int create_matrix(NodePair *HashTable, Element *Element_list, RetHelper *ret, SpiceAnalysis options, gsl_vector ***x){

    Element *current = NULL;
	// double **A=NULL, *b = NULL; // A[nodes_num][elements_num]
    gsl_matrix *A=NULL;
    gsl_permutation *P=NULL;
    gsl_vector *b= NULL;
    // gsl_vector *x = NULL;
    unsigned long m2counter = 0;
	int elements_number=0;
	int i=0, j =0,*s = NULL;
	int hash_p=-1, hash_n=-1;

    if (!Element_list) {
		print_error("equation_make", 4, "Element list head empty");
	}

    b = gsl_vector_calloc(ret->amount_of_nodes+ret->group2_size);
        // b = calloc((ret->amount_of_nodes+ret->group2_size), sizeof(double));
    if(!b){
        printf("Something went wrong with stage 1 memory alloc. Exiting.\n");
        return -1;
    }

    // A = calloc((ret->amount_of_nodes+ret->group2_size), sizeof(double));
    A = gsl_matrix_calloc(ret->amount_of_nodes+ret->group2_size, ret->amount_of_nodes+ret->group2_size);
    if(!A){
        gsl_vector_free(b);
        printf("Something went wrong with stage 1 memory alloc. Exiting.\n");
        return -1;
    }
    x = gsl_vector_calloc(ret->amount_of_nodes+ret->group2_size);
    if(!x){
        gsl_matrix_free(A);
        gsl_vector_free(b);
        return -1;
    }
    
    // for(i=0; i < ((ret->amount_of_nodes+ret->group2_size)); i++){
    //     A[i] = calloc((ret->amount_of_nodes+ret->group2_size),sizeof(double));
    //     if(!A[i]){
    //         printf("Something went wrong with stage 2 memory alloc. Exiting.\n");
    //         for(int j=0; j < i; j++){
    //             free(A[j]);
    //         }
    //         free(A);
    //         free(b);
    //         return -1;
    //     }
    // }
    
    current = Element_list;
    for(current=Element_list;current->next!=NULL; current=current->next){
        hash_p = (find_node_pair(HashTable, current->node_p));
		hash_n = (find_node_pair(HashTable, current->node_n));
        
        switch (current->type_of_element)
        {
        case 'v':{
            m2counter++;
            if(hash_p!=0){
                
                // A[hash_p-1][ret->amount_of_nodes+m2counter-1] = A[hash_p-1][ret->amount_of_nodes+m2counter-1] + 1.0;
                // A[ret->amount_of_nodes+m2counter-1][hash_p-1] = A[ret->amount_of_nodes+m2counter-1][hash_p-1]+1.0;
                
                // b[hash_p-1] = b[hash_p-1]+current->value;
                gsl_matrix_set(A, hash_p-1, ret->amount_of_nodes+m2counter-1, gsl_matrix_get(A, hash_p-1, ret->amount_of_nodes+m2counter-1) + 1);
                gsl_matrix_set(A, ret->amount_of_nodes+m2counter-1, hash_p-1, gsl_matrix_get(A, ret->amount_of_nodes+m2counter-1, hash_p-1) + 1);
                gsl_vector_set(b, hash_p-1, gsl_vector_get(b, hash_p-1) + current->value);
            }
            if(hash_n!=0){
                // A[hash_n-1][ret->amount_of_nodes+m2counter-1] = A[hash_n-1][ret->amount_of_nodes+m2counter-1]-1.0;
                // A[ret->amount_of_nodes+m2counter-1][hash_n-1] = A[ret->amount_of_nodes+m2counter-1][hash_n-1]-1.0;
                // b[hash_n-1] = b[hash_n-1]-current->value;
                gsl_matrix_set(A, hash_n-1, ret->amount_of_nodes+m2counter-1, gsl_matrix_get(A, hash_n-1, ret->amount_of_nodes+m2counter-1) - 1);
                gsl_matrix_set(A, ret->amount_of_nodes+m2counter-1, hash_n-1, gsl_matrix_get(A, ret->amount_of_nodes+m2counter-1, hash_n-1) - 1);
                gsl_vector_set(b, hash_n-1, gsl_vector_get(b, hash_n-1) - current->value);
            }
            break;
        }    
        case 'i':{
            if(hash_p!=0){
                // b[hash_p-1] = b[hash_p-1]-current->value;
                gsl_vector_set(b, hash_p-1, gsl_vector_get(b, hash_p-1) - current->value);
            }
            if(hash_n!=0){
                // b[hash_n-1] = b[hash_n-1]+current->value;
                gsl_vector_set(b, hash_n-1, gsl_vector_get(b, hash_n-1) + current->value);
            }
            break;
        }    
        case 'r':{
            if(hash_p!=0){
                // A[hash_p-1][hash_p-1] = A[hash_p-1][hash_p-1] + (1/current->value);
                gsl_matrix_set(A, hash_p-1, hash_p-1, gsl_matrix_get(A, hash_p-1, hash_p-1) + (1/current->value));
                if(hash_n!=0){
                    // A[hash_n-1][hash_p-1] = A[hash_n-1][hash_p-1]-(1/current->value);
                    // A[hash_p-1][hash_n-1] = A[hash_p-1][hash_n-1] - (1/current->value);
                    gsl_matrix_set(A, hash_n-1, hash_p-1, gsl_matrix_get(A, hash_n-1, hash_p-1) - (1/current->value));
                    gsl_matrix_set(A, hash_p-1, hash_n-1, gsl_matrix_get(A, hash_p-1, hash_n-1) - (1/current->value));
                }
            }
            if(hash_n!=0){
                // A[hash_n-1][hash_n-1] = A[hash_n-1][hash_n-1] + (1/current->value);
                gsl_matrix_set(A, hash_n-1, hash_n-1, gsl_matrix_get(A, hash_n-1, hash_n-1) + (1/current->value));
            }
            break;
        }    
        case 'l':{
            m2counter++;
            if(hash_p!=0){
                // A[hash_p-1][ret->amount_of_nodes+m2counter-1] = A[hash_p-1][ret->amount_of_nodes+m2counter-1] + 1.0;
                // A[ret->amount_of_nodes+m2counter-1][hash_p-1] = A[ret->amount_of_nodes+m2counter-1][hash_p-1]+1.0;
                // b[hash_p-1] = b[hash_p-1]+0;
                gsl_matrix_set(A, hash_p-1, ret->amount_of_nodes+m2counter-1, gsl_matrix_get(A, hash_p-1, ret->amount_of_nodes+m2counter-1) + 1);
                gsl_matrix_set(A, ret->amount_of_nodes+m2counter-1, hash_p-1, gsl_matrix_get(A, ret->amount_of_nodes+m2counter-1, hash_p-1) + 1);
                gsl_vector_set(b, hash_p-1, gsl_vector_get(b, hash_p-1) + 0);
            }
            if(hash_n!=0){
                // A[hash_n-1][ret->amount_of_nodes+m2counter-1] = A[hash_n-1][ret->amount_of_nodes+m2counter-1]-1.0;
                // A[ret->amount_of_nodes+m2counter-1][hash_n-1] = A[ret->amount_of_nodes+m2counter-1][hash_n-1]-1.0;
                // b[hash_n-1] = b[hash_n-1]-0;
                gsl_matrix_set(A, hash_n-1, ret->amount_of_nodes+m2counter-1, gsl_matrix_get(A, hash_n-1, ret->amount_of_nodes+m2counter-1) + 1);
                gsl_matrix_set(A, ret->amount_of_nodes+m2counter-1, hash_n-1, gsl_matrix_get(A, ret->amount_of_nodes+m2counter-1, hash_n-1) + 1);
                gsl_vector_set(b, hash_n-1, gsl_vector_get(b, hash_n-1) + 0);
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


	print_equation_system(*ret, A, b);


    //cholesky solution needs to be tested.
    if(ret->chol_flag == 1){
        gsl_linalg_cholesky_decomp1(A);
        gsl_linalg_cholesky_solve(A, b, x);
        return 0;
    }
    else{
        P = gsl_permutation_calloc(ret->amount_of_nodes+ret->group2_size);
        if(!P){
            gsl_matrix_free(A);
            gsl_vector_free(b);
            gsl_vector_free(x);
            printf("Something went wrong with stage 1 memory alloc. Exiting.\n");
            return -1;
        }
        //LU decomp(needs work)
        gsl_linalg_LU_decomp(A,P,s);//ti vazoume gia signum?
        gsl_linalg_LU_solve(A,P,b,x);
        //filling L and U 
        return 0;
    }
    // printf("A table is: \n");
    // for(i = 0; i < ret->amount_of_nodes+ret->group2_size; i++){
    //     for(int j=0; j < ret->amount_of_nodes+ret->group2_size; j++){
    //         printf("%.5lf ",A[i][j]);
    //     }
    //     printf("\n");
    // }
    // printf("b table is: \n");
    // for(i = 0; i < ret->amount_of_nodes+ret->group2_size; i++){
    //     printf("%.5lf ",b[i]);
    // }
    return 0;
}


void print_equation_system (RetHelper helper, gsl_matrix *A, gsl_vector *B) {
	int i, j;
	
	printf("\n");

	for (i=0;i<helper.amount_of_nodes+helper.group2_size;i++) {
		for (j=0;j<helper.amount_of_nodes+helper.group2_size;j++) {
			if (i<helper.amount_of_nodes && j<helper.amount_of_nodes) {
				printf("%7.2lf ", gsl_matrix_get(A, i, j));
			}
			else if (i>=helper.amount_of_nodes && j<helper.amount_of_nodes) {
				printf("%7.2lf ", gsl_matrix_get(A, i, j));
			}
			else if (i<helper.amount_of_nodes && j>=helper.amount_of_nodes) {
				printf("%7.2lf ", gsl_matrix_get(A, i, j));
			}
			else{
				printf("%7.2lf ", gsl_matrix_get(A, i, j));
			}
		}
		printf(" %7.2lf\n", gsl_vector_get(B, i));
	}
	printf("\n");
}