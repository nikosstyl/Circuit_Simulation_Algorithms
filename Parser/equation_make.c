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
    // x = gsl_vector_calloc(ret->amount_of_nodes+ret->group2_size);
    // if(!x){
    //     gsl_matrix_free(A);
    //     gsl_vector_free(b);
    //     return -1;
    // }
    
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
                memset(&current->position_in_vector_B, i, sizeof(int));
            }
            if(hash_n!=0){
                // A[hash_n-1][ret->amount_of_nodes+m2counter-1] = A[hash_n-1][ret->amount_of_nodes+m2counter-1]-1.0;
                // A[ret->amount_of_nodes+m2counter-1][hash_n-1] = A[ret->amount_of_nodes+m2counter-1][hash_n-1]-1.0;
                // b[hash_n-1] = b[hash_n-1]-current->value;
                gsl_matrix_set(A, hash_n-1, ret->amount_of_nodes+m2counter-1, gsl_matrix_get(A, hash_n-1, ret->amount_of_nodes+m2counter-1) - 1);
                gsl_matrix_set(A, ret->amount_of_nodes+m2counter-1, hash_n-1, gsl_matrix_get(A, ret->amount_of_nodes+m2counter-1, hash_n-1) - 1);
                gsl_vector_set(b, hash_n-1, gsl_vector_get(b, hash_n-1) - current->value);
                memset(&current->position_in_vector_B, i, sizeof(int));
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

    gsl_vector **x_temp=NULL;
    gsl_permutation *p = gsl_permutation_calloc(b->size);
    if (options.DC_OP == true) {
        x_temp = calloc(1, sizeof(gsl_vector*));
        // if (ret->chol_flag == )
        
        if (!p) {
            print_error("equation_solve",3, "P vector failed to alloc");
        }
        int status = gsl_linalg_LU_decomp(A, p, &status);
        if (status) {
            print_error("equation_solve", 3, gsl_strerror(status));
        }
        x_temp[0] = gsl_vector_calloc(b->size);
        status = gsl_linalg_LU_solve(A, p, b, x_temp[0]);
        if (status) {
            print_error("equation_solve", 3, gsl_strerror(status));
        }
    }
    else if (options.DC_OP == false && options.DC_SWEEP){
        double total_steps=(options.DC_SWEEP->end_val - options.DC_SWEEP->start_val)/options.DC_SWEEP->increment;
		int b_pos=-1, status=0;

		b_pos = find_b_pos (options.DC_SWEEP->variable_name, options.DC_SWEEP->variable_type, Element_list);

		if (b_pos == -1) {
			fprintf(stderr, "\n%sElement %c%s not found in netlist!\nAborting DC sweep%s\n", RED, options.DC_SWEEP->variable_type, options.DC_SWEEP->variable_name, RESET);
		}

		x_temp = calloc(((int)total_steps+1), sizeof(gsl_vector *));
		if (!x_temp) {
			print_error("equation_make", 3, "Solutions vector couldn't reallocate");
		}

		for (int step=0;(step<=(int)total_steps) && (b_pos != -1);step++) {
			x_temp[step] = gsl_vector_calloc(b->size);
			
			gsl_vector_set(b, ret->amount_of_nodes+b_pos, options.DC_SWEEP->start_val + step*(options.DC_SWEEP->increment));
			
			status = gsl_linalg_LU_solve(A, p, b, x_temp[step]);
			if (status) {
				print_error("equation_solve", 4, gsl_strerror(status));
			}
		}
    }

    

    *x = x_temp;
	gsl_permutation_free(p);

    return 0;
}


int find_b_pos (char *element_name, char type, Element *head) {
	Element *current = head;

	while (current->next) {
		if (/* (current->position_in_vector_B != 0) &&  */(strcmp(element_name, current->name)==0) && type==current->type_of_element) {
			return(current->position_in_vector_B);
		}
		current = current->next;
	}
	return -1;
}

void print_sols(char* filename, gsl_vector **x, NodePair *pair_head, RetHelper helper, SpiceAnalysis options) {
	double total_steps = 1;
	int step=0, element_index=0;
	FILE *output_file=NULL;
	char str_temp[MAX_CHAR_NUM]={"\0"};

	if (!*x) {
		return;
	}

	// Open output file accordingly
	// Also, determine the total_steps for printing
	if (options.DC_SWEEP) {
		output_file = fopen(strcat(filename, ".dc.out"), "wb");
		total_steps=(options.DC_SWEEP->end_val - options.DC_SWEEP->start_val)/options.DC_SWEEP->increment;
	}
	else {
		output_file = fopen(strcat(filename, ".op.out"), "wb");
		total_steps = 1;
	}

	fprintf(output_file, "(V(name): step_value) | Nothing\tV(to_print): value\n");
	fprintf(output_file, "------------------------------------------------\n");

	// The first time has to be printed seperately because .OP
	if (options.DC_SWEEP) {
		fprintf(output_file, "%c%s: %lf\t", options.DC_SWEEP->variable_type, options.DC_SWEEP->variable_name, options.DC_SWEEP->start_val + options.DC_SWEEP->increment*step);
	}
	
	for (element_index=0;element_index<options.PLOT->str_num;element_index++) {
		int size = strlen(&options.PLOT->elements_to_print[element_index][2])-1;
		
		strncpy(str_temp, &options.PLOT->elements_to_print[element_index][2], size);
		strToLower(str_temp);
		
		int index = find_node_pair(pair_head,str_temp);
		fprintf(output_file, "%c%s: %lf\t", options.PLOT->elements_to_print[element_index][0], str_temp, gsl_vector_get(x[step], index - 1));
	}
	fprintf(output_file, "\n");
	
	if (options.DC_OP == true) {
		return;
	}

	for (step=1;step<=total_steps;step++) {
		if (options.DC_SWEEP) {
			fprintf(output_file, "%c%s: %lf\t", options.DC_SWEEP->variable_type, options.DC_SWEEP->variable_name, options.DC_SWEEP->start_val + options.DC_SWEEP->increment*step);
		}

		for (element_index=0;element_index<options.PLOT->str_num;element_index++) {
			int size = strlen(&options.PLOT->elements_to_print[element_index][2])-1;
			
			strncpy(str_temp, &options.PLOT->elements_to_print[element_index][2], size);
			strToLower(str_temp);
			
			// b_pos = find_b_pos(str_temp, tolower(options.PLOT->elements_to_print[element_index][0]), head);
			int index = find_node_pair(pair_head,str_temp);
			fprintf(output_file, "%c%s: %lf\t", options.PLOT->elements_to_print[element_index][0], str_temp, gsl_vector_get(x[step], index - 1));
		}
		fprintf(output_file, "\n");
	}
}

void print_equation_system (RetHelper helper, gsl_matrix *A, gsl_vector *B) {
	int i, j;
	
	printf("\n");

	for (i=0;i<helper.amount_of_nodes+helper.group2_size;i++) {
		for (j=0;j<helper.amount_of_nodes+helper.group2_size;j++) {
			if (i<helper.amount_of_nodes && j<helper.amount_of_nodes) {
				printf("%s%7.2lf ", RED, gsl_matrix_get(A, i, j));
			}
			else if (i>=helper.amount_of_nodes && j<helper.amount_of_nodes) {
				printf("%s%7.2lf ", BLUE, gsl_matrix_get(A, i, j));
			}
			else if (i<helper.amount_of_nodes && j>=helper.amount_of_nodes) {
				printf("%s%7.2lf ", GREEN, gsl_matrix_get(A, i, j));
			}
			else{
				printf("%s%7.2lf ", YELLOW, gsl_matrix_get(A, i, j));
			}
		}
		printf(" %s%7.2lf\n", RESET, gsl_vector_get(B, i));
	}
	printf("\n");
}