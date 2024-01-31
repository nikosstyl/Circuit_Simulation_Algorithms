#include "parser.h"
#include <stdlib.h>
#include <stdio.h>
#include <gsl/gsl_linalg.h>

// cs_spalloc but with calloc
cs *cs_spcalloc(int m, int n, int nzmax, int values, int triplet) {

	cs *A = (cs *) cs_calloc(1, sizeof(cs)); /* allocate the cs struct */
	if (!A)
		return (NULL); /* out of memory */
	A->m = m; /* define dimensions and nzmax */
	A->n = n;
	A->nzmax = nzmax = CS_MAX (nzmax, 1);
	A->nz = triplet ? 0 : -1; /* allocate triplet or comp.col */
	A->p = (int *) cs_calloc(triplet ? nzmax : n + 1, sizeof(int));
	A->i = (int *) cs_calloc(nzmax, sizeof(int));
	A->x = (double *) (values ? cs_calloc(nzmax, sizeof(double)) : NULL);
	return ((!A->p || !A->i || (values && !A->x)) ? cs_spfree(A) : A);
}


void gsl_vector_multiply(gsl_vector *a, gsl_vector *b, gsl_vector *result) {
    size_t i;
    for (i = 0; i < a->size; i++) {
        double ai = gsl_vector_get(a, i);
        double bi = gsl_vector_get(b, i);
        gsl_vector_set(result, i, ai * bi);
    }
}

int sparse_equation_make(Element *head, NodePair *pair_head, RetHelper helper, cs **A, gsl_vector **b) {
	Element *current = NULL;
	cs *A_temp = NULL, *C_temp = NULL;
	gsl_vector *b_temp;
	unsigned long hash_p=0, hash_n=0;
	int i=0, k=0;


	if (!head) {
		print_error("equation_make", ERR_GENERAL, "Element list head empty");
	}

	// CSparse implementation

	// If cs_spalloc is used, valgrind throws a bunch of errors
	A_temp = cs_spcalloc(helper.amount_of_nodes+helper.group2_size, helper.amount_of_nodes+helper.group2_size, helper.non_zero_elements, 1, 1);
	if (!A_temp) {
		print_error("equation_make", ERR_NO_MEM, "Matrix A_temp couldn't be created");
	}
	
	A_temp->nz = helper.non_zero_elements;

	b_temp = gsl_vector_calloc(helper.amount_of_nodes+helper.group2_size);
	if (!b_temp) {
		print_error("sparse_equation_make", ERR_NO_MEM, "Vector B_gsl couldn't be created");
	}

	// Fill A_temp matrix
	i=0; k=0;
	for (current = head;current->next != NULL;current=current->next) {
		hash_p = find_node_pair(pair_head, current->node_p);
		hash_n = find_node_pair(pair_head, current->node_n);

		switch (current->type_of_element) {
			case 'r': {
				if ((hash_p !=0) && (hash_n != 0)) {
					A_temp->i[k] = hash_p-1;
					A_temp->p[k] = hash_p-1;
					A_temp->x[k] += 1 / current->value;

					k++;
					
					A_temp->i[k] = hash_p-1;
					A_temp->p[k] = hash_n-1;
					A_temp->x[k] -= 1 / current->value;

					k++;

					A_temp->i[k] = hash_n-1;
					A_temp->p[k] = hash_p-1;
					A_temp->x[k] -= 1 / current->value;

					k++;

					A_temp->i[k] = hash_n-1;
					A_temp->p[k] = hash_n-1;
					A_temp->x[k] += 1 / current->value;

					k++;
				}
				else if (hash_n == 0) {
					A_temp->i[k] = hash_p-1;
					A_temp->p[k] = hash_p-1;
					A_temp->x[k] += 1 / current->value;
					k++;
				}
				else {
					A_temp->i[k] = hash_n-1;
					A_temp->p[k] = hash_n-1;
					A_temp->x[k] -= 1 / current->value;
					k++;
				}
				break;
			}
			case 'i': {
				if ((hash_p != 0) && (hash_n != 0)) {
					// b_temp[hash_p-1] -= current->value;
					// b_temp[hash_n-1] += current->value;
					gsl_vector_set(b_temp, hash_p-1, gsl_vector_get(b_temp, hash_p - 1) - current->value);
					gsl_vector_set(b_temp, hash_n-1, gsl_vector_get(b_temp, hash_n - 1) + current->value);
				}
				else if (hash_n == 0) {
					// b_temp[hash_p-1] -= current->value;
					gsl_vector_set(b_temp, hash_p-1, gsl_vector_get(b_temp, hash_p - 1) - current->value);
				}
				else {
					// b_temp[hash_n-1] += current->value;
					gsl_vector_set(b_temp, hash_n-1, gsl_vector_get(b_temp, hash_n - 1) + current->value);
				}
				break;
			}
			case 'l': {
				// For inductors, treat them as 0-volt voltage source
				// in DC analysis (often called and .op)
				// So, it will skip this case and move on to 'v'.
			}
			case 'v': {
				if ((hash_p != 0) && (hash_n != 0)) {
					A_temp->i[k] = helper.amount_of_nodes+i;
					A_temp->p[k] = hash_p-1;
					A_temp->x[k] += 1;
					k++;

					A_temp->i[k] = helper.amount_of_nodes+i;
					A_temp->p[k] = hash_n-1;
					A_temp->x[k] -= 1;
					k++;

					A_temp->i[k] = hash_p-1;
					A_temp->p[k] = helper.amount_of_nodes+i;
					A_temp->x[k] += 1;
					k++;

					A_temp->i[k] = hash_n-1;
					A_temp->p[k] = helper.amount_of_nodes+i;
					A_temp->x[k] -= 1;
					k++;
				}
				else if (hash_n ==0) {
					A_temp->i[k] = helper.amount_of_nodes+i;
					A_temp->p[k] = hash_p-1;
					A_temp->x[k] += 1;
					k++;

					A_temp->i[k] = hash_p-1;
					A_temp->p[k] = helper.amount_of_nodes+i;
					A_temp->x[k] += 1;
					k++;
				}
				else {
					A_temp->i[k] = helper.amount_of_nodes+i;
					A_temp->p[k] = hash_n-1;
					A_temp->x[k] -= 1;
					k++;

					A_temp->i[k] = hash_n-1;
					A_temp->p[k] = helper.amount_of_nodes+i;
					A_temp->x[k] -= 1;
					k++;
				}
				gsl_vector_set(b_temp, helper.amount_of_nodes+i, current->type_of_element=='v' ? (current->value + gsl_vector_get(b_temp, helper.amount_of_nodes+i)): 0);
				current->position_in_vector_B = i;
				i++;
				break;
			}
			case 'c': {
				// Skip for DC analysis
				break;
			}
		}
	}

	C_temp = cs_compress(A_temp);
	if (!C_temp) {
		print_error("equation_make", ERR_GENERAL, "Matrix A_temp couldn't be compressed");
	}

	cs_spfree(A_temp);

	if (!cs_dupl(C_temp)) {
		print_error("equation_make", ERR_GENERAL, "Matrix A_temp couldn't be duplicated");
	}

	*A = C_temp;
	*b = b_temp;
	
	return 1;
}

int create_matrix(NodePair *HashTable, Element *Element_list, RetHelper *ret, SpiceAnalysis options, gsl_vector ***x, char *filename){

    Element *current = NULL;
	// double **A=NULL, *b = NULL; // A[nodes_num][elements_num]
    gsl_matrix *A=NULL;
    gsl_vector *b= NULL;
	cs *A_sparse=NULL;
    // gsl_vector *x = NULL;
    unsigned long m2counter = 0;
	// int elements_number=0;
	// int i=0;
	int hash_p=-1, hash_n=-1;
    char str_to_hash[MAX_CHAR_NUM];
    int hash_dc;
    FILE *output_file=NULL;

    if (!Element_list) {
		print_error("equation_make", 4, "Element list head empty");
	}

    b = gsl_vector_calloc(ret->amount_of_nodes+ret->group2_size);
        // b = calloc((ret->amount_of_nodes+ret->group2_size), sizeof(double));
    if(!b){
        printf("Something went wrong with stage 1 memory alloc. Exiting.\n");
        return -1;
    }
	
	if (ret->sparse == true) {
		sparse_equation_make(Element_list, HashTable, *ret, &A_sparse, &b);	
	}
	else {
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
		m2counter = 0;
		current = Element_list;
		for(current=Element_list;current->next!=NULL; current=current->next){
			hash_p = (find_node_pair(HashTable, current->node_p));
			hash_n = (find_node_pair(HashTable, current->node_n));
			// i=0;
			switch (current->type_of_element)
			{
			case 'v':{
				if(hash_p!=0){
					
					// A[hash_p-1][ret->amount_of_nodes+m2counter-1] = A[hash_p-1][ret->amount_of_nodes+m2counter-1] + 1.0;
					// A[ret->amount_of_nodes+m2counter-1][hash_p-1] = A[ret->amount_of_nodes+m2counter-1][hash_p-1]+1.0;
					
					// b[hash_p-1] = b[hash_p-1]+current->value;
					gsl_matrix_set(A, hash_p-1, ret->amount_of_nodes+m2counter, gsl_matrix_get(A, hash_p-1, ret->amount_of_nodes+m2counter) + 1);
					gsl_matrix_set(A, ret->amount_of_nodes+m2counter, hash_p-1, gsl_matrix_get(A, ret->amount_of_nodes+m2counter, hash_p-1) + 1);
					
				}
				if(hash_n!=0){
					// A[hash_n-1][ret->amount_of_nodes+m2counter-1] = A[hash_n-1][ret->amount_of_nodes+m2counter-1]-1.0;
					// A[ret->amount_of_nodes+m2counter-1][hash_n-1] = A[ret->amount_of_nodes+m2counter-1][hash_n-1]-1.0;
					// b[hash_n-1] = b[hash_n-1]-current->value;
					gsl_matrix_set(A, hash_n-1, ret->amount_of_nodes+m2counter, gsl_matrix_get(A, hash_n-1, ret->amount_of_nodes+m2counter) - 1);
					gsl_matrix_set(A, ret->amount_of_nodes+m2counter, hash_n-1, gsl_matrix_get(A, ret->amount_of_nodes+m2counter, hash_n-1) - 1);
					
				}
				//gsl_vector_set(b, ret->amount_of_nodes+i, gsl_vector_get(b, ret->amount_of_nodes+i) + current->value);
				//memset(&current->position_in_vector_B, i, sizeof(int));
				//i++;
				gsl_vector_set(b, ret->amount_of_nodes+m2counter, gsl_vector_get(b,ret->amount_of_nodes+m2counter)+current->value);
				current->position_in_vector_B = ret->amount_of_nodes+m2counter;
				m2counter++;
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
				if(hash_p!=0){
					// A[hash_p-1][ret->amount_of_nodes+m2counter-1] = A[hash_p-1][ret->amount_of_nodes+m2counter-1] + 1.0;
					// A[ret->amount_of_nodes+m2counter-1][hash_p-1] = A[ret->amount_of_nodes+m2counter-1][hash_p-1]+1.0;
					// b[hash_p-1] = b[hash_p-1]+0;
					gsl_matrix_set(A, hash_p-1, ret->amount_of_nodes+m2counter, gsl_matrix_get(A, hash_p-1, ret->amount_of_nodes+m2counter) + 1);
					gsl_matrix_set(A, ret->amount_of_nodes+m2counter, hash_p-1, gsl_matrix_get(A, ret->amount_of_nodes+m2counter, hash_p-1) + 1);
					
				}
				if(hash_n!=0){
					// A[hash_n-1][ret->amount_of_nodes+m2counter-1] = A[hash_n-1][ret->amount_of_nodes+m2counter-1]-1.0;
					// A[ret->amount_of_nodes+m2counter-1][hash_n-1] = A[ret->amount_of_nodes+m2counter-1][hash_n-1]-1.0;
					// b[hash_n-1] = b[hash_n-1]-0;
					gsl_matrix_set(A, hash_n-1, ret->amount_of_nodes+m2counter, gsl_matrix_get(A, hash_n-1, ret->amount_of_nodes+m2counter) + 1);
					gsl_matrix_set(A, ret->amount_of_nodes+m2counter, hash_n-1, gsl_matrix_get(A, ret->amount_of_nodes+m2counter, hash_n-1) + 1);
					// gsl_vector_set(b, hash_n-1, gsl_vector_get(b, hash_n-1) + 0);
				}
				//gsl_vector_set(b, ret->amount_of_nodes+i, gsl_vector_get(b, ret->amount_of_nodes+i) + 0);
				// memset(&current->position_in_vector_B, i, sizeof(int));
				// i++;
				gsl_vector_set(b, ret->amount_of_nodes+m2counter, gsl_vector_get(b,ret->amount_of_nodes+m2counter)+0);
				current->position_in_vector_B = ret->amount_of_nodes+m2counter;
				m2counter++;
				break;
			}    
			case 'c':
				break;        
			default:
				break;
			}
			//current = current->next;
			// printf("m2counter: %lu\n", m2counter);
		}
	}


	// print_equation_system(*ret, A, b);
	gsl_vector **x_temp=NULL;

	if (ret->sparse) {
		if (ret->use_iterations || ret->use_iterations_cg) {
			sparse_iterative_equation_solve(A_sparse, b, &x_temp, options, Element_list, *ret, HashTable);
		}
		else {
			sparse_direct_equation_solve(A_sparse, b, &x_temp, options, Element_list, *ret, HashTable);
		}
	}
	else {
		int status;
		gsl_permutation *p = NULL;

		if (!ret->direct_chol_flag && (!ret->use_iterations &&  !ret->use_iterations_cg)) {
			p = gsl_permutation_calloc(b->size);
			if (!p) {
				print_error("equation_solve",3, "P vector failed to alloc");
			}

			status = gsl_linalg_LU_decomp(A, p, &status);
			if (status) {
				print_error("equation_solve", 3, gsl_strerror(status));
			}
		}
		else if (ret->direct_chol_flag){
			status = gsl_linalg_cholesky_decomp1(A);
			if (status) {
				print_error("equation_solve", 4, gsl_strerror(status));
			}
		}

		if (options.DC_OP == true) {
			x_temp = calloc(1, sizeof(gsl_vector*));

			x_temp[0] = gsl_vector_calloc(b->size);
			
			if (ret->use_iterations) {
				bicg_solve(A, b, &x_temp[0], ret->tolerance, A->size1);
			}
			else if (ret->use_iterations_cg) {
				cg_solve(A, b, &x_temp[0], ret->tolerance, A->size1);
			}
			else if (ret->direct_chol_flag == 0) {
				status = gsl_linalg_LU_solve(A, p, b, x_temp[0]);
				if (status) {
					print_error("equation_solve", 3, gsl_strerror(status));
				}
			}
			else {
				status = gsl_linalg_cholesky_solve(A, b, x_temp[0]);
				if (status) {
					print_error("equation_solve", 4, gsl_strerror(status));
				}
				printf("CHOLESKY USED!\n");
			}
		}
		else if (options.DC_OP == false && options.DC_SWEEP){
			double total_steps=(options.DC_SWEEP->end_val - options.DC_SWEEP->start_val)/options.DC_SWEEP->increment;
			int b_pos=-1, status=0;

			int b_pos_arr[2]={0};
			b_pos = find_b_pos (options.DC_SWEEP->variable_name, options.DC_SWEEP->variable_type, Element_list, b_pos_arr, HashTable);

			if (b_pos == -1) {
				fprintf(stderr, "\n%sElement %c%s not found in netlist!\nAborting DC sweep%s\n", RED, options.DC_SWEEP->variable_type, options.DC_SWEEP->variable_name, RESET);
			}

			x_temp = calloc(((int)total_steps+1), sizeof(gsl_vector *));
			if (!x_temp) {
				print_error("equation_make", 3, "Solutions vector couldn't reallocate");
			}

			for (int step=0;(step<=(int)total_steps) && (b_pos != -1);step++) {
				x_temp[step] = gsl_vector_calloc(b->size);
				// printf("BPOS IS: %d\n",b_pos);

				if (b_pos != -2) {
					gsl_vector_set(b, b_pos, options.DC_SWEEP->start_val + step*(options.DC_SWEEP->increment));
				}
				else {
					if (b_pos_arr[0] != 0) {
						gsl_vector_set(b, b_pos_arr[0]-1, -options.DC_SWEEP->start_val - step*(options.DC_SWEEP->increment));
					}
					if (b_pos_arr[1] != 0) {
						gsl_vector_set(b, b_pos_arr[1]-1, options.DC_SWEEP->start_val + step*(options.DC_SWEEP->increment));
					}
				}
				
				if (ret->use_iterations) {
					// fprintf(stderr, "Tolerance: %lf\n", ret->tolerance);
					bicg_solve(A, b, &x_temp[step], ret->tolerance, A->size1);
				}
				else if (ret->use_iterations_cg) {
					cg_solve(A, b, &x_temp[step], ret->tolerance, A->size1);
				}
				else if (ret->direct_chol_flag == 0) {
					status = gsl_linalg_LU_solve(A, p, b, x_temp[step]);
					if (status) {
						print_error("equation_solve", 4, gsl_strerror(status));
					}
				}
				else {
					status = gsl_linalg_cholesky_solve(A, b, x_temp[step]);
					if (status) {
						print_error("equation_solve", 4, gsl_strerror(status));
					}
					// printf("CHOLESKY USED!\n");
				}
			}
		}
		
		// *x = x_temp;
		gsl_permutation_free(p);
	}

    if(options.PLOT) {
        double total_steps=0;

        if (options.DC_SWEEP) {
            output_file = fopen(strcat(filename, ".dc.out"), "wb");
            total_steps=(options.DC_SWEEP->end_val - options.DC_SWEEP->start_val)/options.DC_SWEEP->increment;
        }
        else {
            output_file = fopen(strcat(filename, ".op.out"), "wb");
            total_steps = 0;
        }
        
        // printf("\nDC SWEEP\n");
		for(int step=0; step<=(int)total_steps;step++) {
            fprintf(output_file, "Step %d:\t", step+1);
            for(int i=0; i<options.PLOT->str_num; i++) {
                int size = strlen(&options.PLOT->elements_to_print[i][2])-1;
                // memcpy(str_to_hash, "\0", sizeof(char)*MAX_CHAR_NUM);
                for (int j=0; j<MAX_CHAR_NUM; j++) {
                    str_to_hash[j] = '\0';
                }
                strncpy(str_to_hash, &options.PLOT->elements_to_print[i][2], size);
                strToLower(str_to_hash);
                hash_dc = find_node_pair(HashTable, str_to_hash);
                // printf("STEP: %d\n", step);
                // printf("HASH_DC: %d\n", hash_dc);
                fprintf(output_file, "V(%s) %lf\t", str_to_hash, gsl_vector_get(x_temp[step], hash_dc-1));
            }
            fprintf(output_file, "\n");
		}
		fflush(output_file);

		if (options.DC_SWEEP) {
        	plot("Circuit Simulation Algorithms", x_temp, HashTable, *ret, options);
		}
    }


	*x = x_temp;

    return 0;
}


int find_b_pos (char *element_name, char type, Element *head, int* out, NodePair *HashTable) {
	// out[0] = hash_p; out[1] = hash_n;
    Element *current = head;

    if (type == 'v') {
        while (current->next) {
            if ((strcmp(element_name, current->name)==0) && type==current->type_of_element) {
                return(current->position_in_vector_B);
            }
            current = current->next;
        }
    }
    else if (type == 'i') {
        while (current->next) {
            if ((strcmp(element_name, current->name)==0) && type==current->type_of_element) {
                out[0] = find_node_pair(HashTable, current->node_p);
                out[1] = find_node_pair(HashTable, current->node_n);
                return -2;
            }
            current = current->next;
        }
    }
	return -1;
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

void sparse_direct_equation_solve(cs *A, gsl_vector *B, gsl_vector ***x, SpiceAnalysis options, Element *head, RetHelper helper, NodePair *pair_head) {
	css *S = NULL;
	csn *N = NULL;
	gsl_vector **x_temp = NULL, *temp = NULL;

	if (!A || !B) {
		print_error("sparse_direct_equation_solve", ERR_GENERAL, "Matrix A or B are NULL");
	}

	if (helper.direct_chol_flag == false) {
		S = cs_sqr(2, A, 0);
		if (!S) {
			print_error("sparse_direct_equation_solve", ERR_GENERAL, "S Matrix not able to calloc");
		}
		N = cs_lu(A, S, 1);
		if (!N) {
			print_error("sparse_direct_equation_solve", ERR_GENERAL, "N Matrix not able to calloc");
		}
	}
	else {
		S = cs_schol(1, A);
		if (!S) {
			print_error("sparse_direct_equation_solve", ERR_GENERAL, "S Matrix not able to calloc");
		}
		N = cs_chol(A, S);
		if (!N) {
			print_error("sparse_direct_equation_solve", ERR_GENERAL, "N Matrix not able to calloc");
		}
	}

	if (options.DC_OP == true) {
		x_temp = calloc(1, sizeof(gsl_vector *));
		if (!x_temp) {
			print_error("equation_make", ERR_NO_MEM, "Sols Matrix not able to calloc");
		}

		x_temp[0] = gsl_vector_calloc(B->size);
		temp = gsl_vector_calloc(B->size);
		// temp = calloc(helper.node_num+helper.m2, sizeof(double)); // Calloc a temp vector

		if (helper.direct_chol_flag == false) {
			// LU Decomp
			cs_ipvec(N->pinv, B->data, temp->data, helper.amount_of_nodes+helper.group2_size);
			cs_lsolve(N->L, temp->data);
			cs_usolve(N->U, temp->data);
			cs_ipvec(S->q, temp->data, x_temp[0]->data, helper.amount_of_nodes+helper.group2_size);
		}
		else {
			// Cholesky decomposition
			cs_ipvec(S->pinv, B->data, temp->data, helper.amount_of_nodes+helper.group2_size);
			cs_lsolve(N->L, temp->data);
			cs_ltsolve(N->L, temp->data);
			cs_pvec(S->pinv, temp->data, x_temp[0]->data, helper.amount_of_nodes+helper.group2_size);
		}
		gsl_vector_free(temp);
	}
	else if ((options.DC_OP == false) && (options.DC_SWEEP)) { // For each iteration, B vector has to change
		double total_steps=(options.DC_SWEEP->end_val - options.DC_SWEEP->start_val)/options.DC_SWEEP->increment;
		int b_pos=-1;
		int find_pos_ret[2]={0};

		b_pos = find_b_pos(options.DC_SWEEP->variable_name, options.DC_SWEEP->variable_type, head, find_pos_ret, pair_head);

		if (find_pos_ret[0] == -1 || find_pos_ret[1] == -1 || b_pos == -1) {
			fprintf(stderr, "\n%sElement %c%s not found in netlist!\nAborting DC sweep%s\n", RED, options.DC_SWEEP->variable_type, options.DC_SWEEP->variable_name, RESET);
			return;
		}

		x_temp = calloc(((int)total_steps+1), sizeof(gsl_vector *));
		if (!x_temp) {
			print_error("equation_make", ERR_NO_MEM, "Solutions vector couldn't reallocate");
		}

		for (int step=0;step<=(int)total_steps;step++) {
			x_temp[step] = gsl_vector_calloc(B->size);

			temp = gsl_vector_calloc(helper.amount_of_nodes+helper.group2_size);

			if (options.DC_SWEEP->variable_type == 'v') {
				gsl_vector_set(B, helper.amount_of_nodes + b_pos, options.DC_SWEEP->start_val + step*(options.DC_SWEEP->increment));
			}
			else if( options.DC_SWEEP->variable_type == 'i') {
				if (find_pos_ret[0] > 0) {
					gsl_vector_set(B, find_pos_ret[0]-1, -(options.DC_SWEEP->start_val + step*(options.DC_SWEEP->increment)));
				}
				if (find_pos_ret[1] > 0) {
					gsl_vector_set(B, find_pos_ret[1]-1, options.DC_SWEEP->start_val + step*(options.DC_SWEEP->increment));
				}
			}
			
			if (helper.direct_chol_flag == false) {
				cs_ipvec(N->pinv, B->data, temp->data, helper.amount_of_nodes+helper.group2_size);
				cs_lsolve(N->L, temp->data);
				cs_usolve(N->U, temp->data);
				cs_ipvec(S->q, temp->data, x_temp[step]->data, helper.amount_of_nodes+helper.group2_size);
			}
			else {
				cs_ipvec(S->pinv, B->data, temp->data, helper.amount_of_nodes+helper.group2_size);
				cs_lsolve(N->L, temp->data);
				cs_ltsolve(N->L, temp->data);
				cs_pvec(S->pinv, temp->data, x_temp[step]->data, helper.amount_of_nodes+helper.group2_size);
			}
			gsl_vector_free(temp);
		}
	}
	*x = x_temp;

	cs_nfree(N);
	cs_sfree(S);
}

void sparse_iterative_equation_solve (const cs *A, gsl_vector *B, gsl_vector ***x, SpiceAnalysis options, Element *head, RetHelper helper, NodePair *pair_head) {
	gsl_vector **x_temp = NULL;

	if (!A || !B) {
		print_error("sparse_iterative_equation_solve", ERR_GENERAL, "A or B is NULL");
	} 

	if (options.DC_OP) {
		x_temp = calloc(1, sizeof(gsl_vector *));
		if (!x_temp) {
			print_error("iterative_equation_solve", ERR_NO_MEM, "x_temp not able to calloc");
		}
		// x_temp[0] = calloc(A->n, sizeof(double));
		x_temp[0] = gsl_vector_calloc(B->size);
		if (!x_temp[0]) {
			print_error("iterative_equation_solve", ERR_NO_MEM, "x_temp[0] not able to calloc");
		}

		if (helper.use_iterations_cg) {
			sparse_cg_iter(A, B, &x_temp[0], helper.tolerance);
		}
		else if (helper.use_iterations) {
			sparse_bi_cg_iter(A, B, &x_temp[0], helper.tolerance);
		}
	}
	else if (options.DC_OP == false && options.DC_SWEEP) {
		double total_steps=(options.DC_SWEEP->end_val - options.DC_SWEEP->start_val)/options.DC_SWEEP->increment;
		int b_pos=-1;
		int find_pos_ret[2]={0};

		b_pos = find_b_pos (options.DC_SWEEP->variable_name, options.DC_SWEEP->variable_type, head, find_pos_ret, pair_head);

		if (find_pos_ret[0] == -1 || find_pos_ret[1] == -1 || b_pos == -1) {
			fprintf(stderr, "\n%sElement %c%s not found in netlist!\nAborting DC sweep%s\n", RED, options.DC_SWEEP->variable_type, options.DC_SWEEP->variable_name, RESET);
		}

		x_temp = calloc(((int)total_steps+1), sizeof(gsl_vector *));
		if (!x_temp) {
			print_error("equation_make", ERR_NO_MEM, "Solutions vector couldn't reallocate");
		}

		for (int step=0;step<=(int)total_steps;step++) {
			x_temp[step] = gsl_vector_calloc(B->size);
			
			if (options.DC_SWEEP->variable_type == 'v') {
				gsl_vector_set(B, helper.amount_of_nodes+b_pos, options.DC_SWEEP->start_val + step*(options.DC_SWEEP->increment));
			}
			else if( options.DC_SWEEP->variable_type == 'i') {
				if (find_pos_ret[0] > 0) {
					gsl_vector_set(B, find_pos_ret[0]-1, -(options.DC_SWEEP->start_val + step*(options.DC_SWEEP->increment)));
				}
				if (find_pos_ret[1] > 0) {
					gsl_vector_set(B, find_pos_ret[1]-1, options.DC_SWEEP->start_val + step*(options.DC_SWEEP->increment));
				}
			}
			
			if (helper.use_iterations_cg) {
				sparse_cg_iter(A, B, &x_temp[step], helper.tolerance);
			}
			else if (helper.use_iterations) {
				sparse_bi_cg_iter(A, B, &x_temp[step], helper.tolerance);
			}
		}
	}
	*x = x_temp;
}

void cg_solve(gsl_matrix *A, gsl_vector *b, gsl_vector **x, double itol, int n) {
    int iter = 0;
    gsl_vector *r = gsl_vector_alloc(b->size);
    gsl_vector *z = gsl_vector_alloc(b->size);
    gsl_vector *p = gsl_vector_alloc(b->size);
    gsl_vector *q = gsl_vector_alloc(b->size);
    gsl_matrix *M = gsl_matrix_calloc(A->size1, A->size2);
    double rho, rho1, beta, alpha, diagElement, b_norm;

    gsl_blas_dgemv(CblasNoTrans, -1.0, A, *x, 0.0, r);
    gsl_vector_add(r, b); // r = b - Ax

    b_norm = gsl_blas_dnrm2(b);
    if(b_norm == 0) {
        b_norm = 1;
    }
    while (gsl_blas_dnrm2(r) / b_norm > itol && iter < n) {
        iter++;

        for (int i = 0; i < A->size1; i++) {
            diagElement = gsl_matrix_get(A, i, i);
            if(diagElement == 0) {
                diagElement = 1.0;
            }
            gsl_matrix_set(M, i, i, 1.0 / diagElement);
        }
        gsl_blas_dgemv(CblasNoTrans, 1.0, M, r, 0.0, z);

        gsl_blas_ddot(r, z, &rho); // rho = rT * z

        if (iter == 1) {
            gsl_vector_memcpy(p, z); // p = z
        } else {
            beta = rho / rho1;
            gsl_vector_scale(p, beta);
            gsl_vector_add(p, z); // p = z + beta * p
        }

        gsl_blas_dgemv(CblasNoTrans, 1.0, A, p, 0.0, q); // q = Ap

        gsl_blas_ddot(p, q, &alpha); // alpha = pT * q
        alpha = rho / alpha;

        gsl_blas_daxpy(alpha, p, *x); // x = x + alpha * p
        gsl_blas_daxpy(-alpha, q, r); // r = r - alpha * q

        rho1 = rho;

        b_norm = gsl_blas_dnrm2(b);
        if(b_norm == 0) {
            b_norm = 1;
        }
    }

    gsl_vector_free(r);
    gsl_vector_free(z);
    gsl_vector_free(p);
    gsl_vector_free(q);
    gsl_matrix_free(M);
}

void bicg_solve(gsl_matrix *A, gsl_vector *b, gsl_vector **x, double itol, int n) {
    int iter = 0;
    gsl_vector *r = gsl_vector_alloc(b->size);
    gsl_vector *r_tilde = gsl_vector_alloc(b->size);
    gsl_vector *z = gsl_vector_alloc(b->size);
    gsl_vector *z_tilde = gsl_vector_alloc(b->size);
    gsl_vector *p = gsl_vector_alloc(b->size);
    gsl_vector *p_tilde = gsl_vector_alloc(b->size);
    gsl_vector *q = gsl_vector_alloc(b->size);
    gsl_vector *q_tilde = gsl_vector_alloc(b->size);
    gsl_matrix *M = gsl_matrix_calloc(A->size1, A->size2);
    double rho, rho1, beta, omega, alpha, diagElement, b_norm;

    gsl_blas_dgemv(CblasNoTrans, -1.0, A, *x, 0.0, r);
    gsl_vector_add(r, b); // r = b - Ax
    gsl_vector_memcpy(r_tilde, r); // r_tilde = r

    b_norm = gsl_blas_dnrm2(b);
    if(b_norm == 0) {
        b_norm = 1;
    }
    while (gsl_blas_dnrm2(r) / b_norm > itol && iter < n) {
        iter++;

        for (int i = 0; i < A->size1; i++) {
            diagElement = gsl_matrix_get(A, i, i);
            if(diagElement == 0) {
                diagElement = 1.0;
            }
            gsl_matrix_set(M, i, i, 1.0 / diagElement); // inverse of M
        }
        gsl_blas_dgemv(CblasNoTrans, 1.0, M, r, 0.0, z);
        // for a diagonal matrix, the transpose of the inverse is the same as the inverse of the transpose
        gsl_blas_dgemv(CblasNoTrans, 1.0, M, r_tilde, 0.0, z_tilde); // we use the same matrix M 

        gsl_blas_ddot(r_tilde, z, &rho); // rho = r_tilde^T * z

        if (fabs(rho) < EPS) break; // algorithm failure

        if (iter == 1) {
            gsl_vector_memcpy(p, z); // p = z
            gsl_vector_memcpy(p_tilde, z_tilde); // p_tilde = z_tilde
        } else {
            beta = rho / rho1;
            gsl_vector_scale(p, beta);
            gsl_vector_add(p, z); // p = z + beta * p
            gsl_vector_scale(p_tilde, beta);
            gsl_vector_add(p_tilde, z_tilde); // p_tilde = z_tilde + beta * p_tilde
        }

        gsl_blas_dgemv(CblasNoTrans, 1.0, A, p, 0.0, q); // q = Ap
        gsl_blas_dgemv(CblasTrans, 1.0, A, p_tilde, 0.0, q_tilde); // q_tilde = A^T p_tilde

        gsl_blas_ddot(p_tilde, q, &omega); // omega = p_tilde^T * q

        if (fabs(omega) < EPS) break; // algorithm failure

        alpha = rho / omega;

        gsl_blas_daxpy(alpha, p, *x); // x = x + alpha * p
        gsl_blas_daxpy(-alpha, q, r); // r = r - alpha * q
        gsl_blas_daxpy(-alpha, q_tilde, r_tilde); // r_tilde = r_tilde - alpha * q_tilde

        rho1 = rho;

        b_norm = gsl_blas_dnrm2(b);
        if(b_norm == 0) {
            b_norm = 1;
        }
    }

    gsl_vector_free(r);
    gsl_vector_free(r_tilde);
    gsl_vector_free(z);
    gsl_vector_free(z_tilde);
    gsl_vector_free(p);
    gsl_vector_free(p_tilde);
    gsl_vector_free(q);
    gsl_vector_free(q_tilde);
    gsl_matrix_free(M);
}

void sparse_cg_iter (const cs *A, const gsl_vector *b, gsl_vector **x, double itol) {
	int iter = 0;
	gsl_vector *r = gsl_vector_calloc(b->size);
	gsl_vector *z = gsl_vector_calloc(b->size);
	gsl_vector *p = gsl_vector_calloc(b->size);
	gsl_vector *q = gsl_vector_calloc(b->size);
	gsl_vector *M = gsl_vector_calloc(b->size);
	double rho, rho1=0, beta, alpha, diagElement, b_norm;
	
	cs_gaxpy(A, (*x)->data, r->data);
	gsl_vector_scale(r, -1.0);
	gsl_vector_add(r, b); // r = b - Ax


	b_norm = gsl_blas_dnrm2(b);
	if(b_norm == 0) {
		b_norm = 1;
	}

	gsl_vector_set_all(M, 1);

	for (int i = 0; i < A->n; i++) {
		for (int k=A->p[i];k<A->p[i+1];k++) {
			if (A->i[k] == i) {
				diagElement = A->x[k];
				if(diagElement == 0) {
					diagElement = 1.0;
				}
				gsl_vector_set(M, i, 1.0 / diagElement);
			}
		}
	}

	while (gsl_blas_dnrm2(r) / b_norm > itol && iter < MAX_ITERATIONS) {
		iter++;

		
		gsl_vector_multiply(M, r, z);

		gsl_blas_ddot(r, z, &rho); // rho = rT * z

		if (iter == 1) {
			gsl_vector_memcpy(p, z); // p = z
		} else {
			beta = rho / rho1;
			gsl_vector_scale(p, beta);
			gsl_vector_add(p, z); // p = z + beta * p
		}

		// gsl_blas_dgemv(CblasNoTrans, 1.0, A, p, 0.0, q); // q = Ap
		// Change the above function to a sparse one.
		gsl_vector_set_all(q, 0.0);
		cs_gaxpy(A, p->data, q->data); // q = Ap

		gsl_blas_ddot(p, q, &alpha); // alpha = pT * q
		alpha = rho / alpha;

		gsl_blas_daxpy(alpha, p, *x); // x = x + alpha * p
		gsl_blas_daxpy(-alpha, q, r); // r = r - alpha * q

		rho1 = rho;

		b_norm = gsl_blas_dnrm2(b);
		if(b_norm == 0) {
			b_norm = 1;
		}
	}

	gsl_vector_free(r);
	gsl_vector_free(z);
	gsl_vector_free(p);
	gsl_vector_free(q);
	gsl_vector_free(M);
}


void sparse_bi_cg_iter (const cs *A, const gsl_vector *b, gsl_vector **x, double itol) {
	int iter = 0;
	gsl_vector *r = gsl_vector_calloc(b->size);
	gsl_vector *r_tilde = gsl_vector_calloc(b->size);
	gsl_vector *z = gsl_vector_calloc(b->size);
	gsl_vector *z_tilde = gsl_vector_calloc(b->size);
	gsl_vector *p = gsl_vector_calloc(b->size);
	gsl_vector *p_tilde = gsl_vector_calloc(b->size);
	gsl_vector *q = gsl_vector_calloc(b->size);
	gsl_vector *q_tilde = gsl_vector_calloc(b->size);
	gsl_vector *M = gsl_vector_calloc(b->size);
	double rho, rho1=0, beta, omega, alpha, diagElement, b_norm;

	// mine_csgaxpy(A, *x, r);
	cs_gaxpy(A, (*x)->data, r->data);
	gsl_vector_scale(r, -1.0);
	gsl_vector_add(r, b); // r = b - Ax
	// gsl_vector_fprintf(stderr, r, "%f");

	gsl_vector_memcpy(r_tilde, r); // r_tilde = r

	b_norm = gsl_blas_dnrm2(b);
	if(b_norm == 0) {
		b_norm = 1;
	}

	gsl_vector_set_all(M, 1);

	for (int j=0;j<A->n;j++) {
		for (int k=A->p[j];k<A->p[j+1];k++) {
			if (A->i[k] == j) {
				diagElement = A->x[k];
				gsl_vector_set(M, j, 1.0 / diagElement);
			}
		}
	}

	while (gsl_blas_dnrm2(r) / b_norm > itol && iter < MAX_ITERATIONS) {
		iter++;

		gsl_vector_multiply(M, r, z);
		// for a diagonal matrix, the transpose of the inverse is the same as the inverse of the transpose
		gsl_vector_multiply(M, r_tilde, z_tilde); // we use the same matrix M 

		gsl_blas_ddot(r_tilde, z, &rho); // rho = r_tilde^T * z

		if (fabs(rho) < GSL_DBL_EPSILON) break; // algorithm failure

		if (iter == 1) {
			gsl_vector_memcpy(p, z); // p = z
			gsl_vector_memcpy(p_tilde, z_tilde); // p_tilde = z_tilde
		} else {
			beta = rho / rho1;
			gsl_vector_scale(p, beta);
			gsl_vector_add(p, z); // p = z + beta * p
			gsl_vector_scale(p_tilde, beta);
			gsl_vector_add(p_tilde, z_tilde); // p_tilde = z_tilde + beta * p_tilde
		}

		gsl_vector_set_all(q, 0.0);
		gsl_vector_set_all(q_tilde, 0.0);
		cs_gaxpy(A, p->data, q->data); // q = Ap
		cs *A_tran = cs_transpose(A, 1);
		cs_gaxpy(A_tran, p_tilde->data, q_tilde->data);
		cs_spfree(A_tran);

		gsl_blas_ddot(p_tilde, q, &omega); // omega = p_tilde^T * q

		if (fabs(omega) < GSL_DBL_EPSILON) break; // algorithm failure

		alpha = rho / omega;

		gsl_blas_daxpy(alpha, p, *x); // x = x + alpha * p
		gsl_blas_daxpy(-alpha, q, r); // r = r - alpha * q
		gsl_blas_daxpy(-alpha, q_tilde, r_tilde); // r_tilde = r_tilde - alpha * q_tilde

		rho1 = rho;

		b_norm = gsl_blas_dnrm2(b);
		if(b_norm == 0) {
			b_norm = 1;
		}
	}

	gsl_vector_free(r);
	gsl_vector_free(r_tilde);
	gsl_vector_free(z);
	gsl_vector_free(z_tilde);
	gsl_vector_free(p);
	gsl_vector_free(p_tilde);
	gsl_vector_free(q);
	gsl_vector_free(q_tilde);
	gsl_vector_free(M);
}

void plot(char *analysis_name, gsl_vector **x, NodePair *pair_head, RetHelper helper, SpiceAnalysis options) {
	FILE *gnupipe = NULL;
	FILE *fdata = NULL;
	int step=0;
	double total_steps;
	char str_temp[MAX_CHAR_NUM];
	// char *GnuPlotArgs[] = {"set grid", "plot \"data.tmp\" with lines"};

	FILE *check = fopen("/usr/bin/gnuplot", "r");
	if (!check) {
		fprintf(stderr, "%s\nGnuPlot is not installed!\n\tInstall it and try again plotting!%s\n", RED, RESET);
		fclose(check);
		return;
	}
	fclose(check);

	if (options.DC_SWEEP) {
		total_steps=(options.DC_SWEEP->end_val - options.DC_SWEEP->start_val)/options.DC_SWEEP->increment;
	}
	else {
		total_steps = 0;
	}

	fdata = fopen("data.tmp", "w");
	// gnupipe = fopen("commands.gnuplot", "w");
	gnupipe = popen("gnuplot -persistent", "w");

	for (step=0;step<=(int)total_steps;step++) {
		for (int element_index=0;element_index<options.PLOT->str_num;element_index++) {
			int size = strlen(&options.PLOT->elements_to_print[element_index][2])-1;
			for (int i=0;i<MAX_CHAR_NUM;i++) {
				str_temp[i] = '\0';
			}
			strncpy(str_temp, &options.PLOT->elements_to_print[element_index][2], size);
			strToLower(str_temp);
			int index = find_node_pair(pair_head,str_temp);
			
			fprintf(fdata, "%lf %lf ", options.DC_SWEEP->start_val + options.DC_SWEEP->increment*step, gsl_vector_get(x[step], index - 1));
		}
		fprintf(fdata, "\n");
		fflush(fdata);
	}

	fprintf(gnupipe, "set term wxt title '%s'\n", analysis_name);
	fprintf(gnupipe, "set grid\n");
	fprintf(gnupipe, "set termoption noenhanced\n");
	fprintf(gnupipe, "set xlabel \"%c(%s)\"\n", toupper(options.DC_SWEEP->variable_type), options.DC_SWEEP->variable_name);
	fprintf(gnupipe, "plot \"data.tmp\" ");

	for (int i=0, j=1;i<options.PLOT->str_num;i++, j+=2) {
		if (i==0) {
			fprintf(gnupipe, "using %d:%d with lines title \"%s\"", j, j+1, options.PLOT->elements_to_print[i]);
		}
		else{
			fprintf(gnupipe, ", \"\" using %d:%d with lines title \"%s\"", j, j+1, options.PLOT->elements_to_print[i]);
		}
	}
	fprintf(gnupipe, "\n");

	fflush(fdata);
	fflush(gnupipe);
	fclose(fdata);
	pclose(gnupipe);

	remove("data.tmp");
}