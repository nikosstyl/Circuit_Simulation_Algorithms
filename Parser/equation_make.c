#include "parser.h"
#include <stdlib.h>
#include <stdio.h>
#include <gsl/gsl_linalg.h>

int create_matrix(NodePair *HashTable, Element *Element_list, RetHelper *ret, SpiceAnalysis options, gsl_vector ***x, char *filename){

    Element *current = NULL;
	// double **A=NULL, *b = NULL; // A[nodes_num][elements_num]
    gsl_matrix *A=NULL;
    gsl_vector *b= NULL;
    // gsl_vector *x = NULL;
    unsigned long m2counter = 0;
	// int elements_number=0;
	int i=0;
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
        i=0;
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


	print_equation_system(*ret, A, b);

    gsl_vector **x_temp=NULL;
    int status;
    gsl_permutation *p = NULL;

    if (ret->chol_flag == false) {
        p = gsl_permutation_calloc(b->size);
        if (!p) {
            print_error("equation_solve",3, "P vector failed to alloc");
        }

        status = gsl_linalg_LU_decomp(A, p, &status);
        if (status) {
            print_error("equation_solve", 3, gsl_strerror(status));
        }
    }
    else {
        status = gsl_linalg_cholesky_decomp1(A);
		if (status) {
			print_error("equation_solve", 4, gsl_strerror(status));
		}
    }

    if (options.DC_OP == true) {
        x_temp = calloc(1, sizeof(gsl_vector*));

        x_temp[0] = gsl_vector_calloc(b->size);
        
        if (ret->chol_flag == false) {
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
			
            if (ret->chol_flag == false) {
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
                printf("CHOLESKY USED!\n");
            }
		}
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
    }

    *x = x_temp;
	gsl_permutation_free(p);

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

void cg_solve(gsl_matrix *A, gsl_vector *b, gsl_vector *x, double itol, int n) {
    int iter = 0;
    gsl_vector *r = gsl_vector_alloc(b->size);
    gsl_vector *z = gsl_vector_alloc(b->size);
    gsl_vector *p = gsl_vector_alloc(b->size);
    gsl_vector *q = gsl_vector_alloc(b->size);
    gsl_matrix *M = gsl_matrix_calloc(A->size1, A->size2);
    double rho, rho1, beta, alpha, diagElement;

    gsl_blas_dgemv(CblasNoTrans, -1.0, A, x, 0.0, r);
    gsl_vector_add(r, b); // r = b - Ax

    while (gsl_blas_dnrm2(r) / gsl_blas_dnrm2(b) > itol && iter < n) {
        iter++;

        for (int i = 0; (i < A->size1) || (i < A->size2); ++i) {
            diagElement = gsl_matrix_get(A, i, i);
            if(diagElement == 0) {
                diagElement = 1;
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

        gsl_blas_daxpy(alpha, p, x); // x = x + alpha * p
        gsl_blas_daxpy(-alpha, q, r); // r = r - alpha * q

        rho1 = rho;
    }

    gsl_vector_free(r);
    gsl_vector_free(z);
    gsl_vector_free(p);
    gsl_vector_free(q);
    gsl_matrix_free(M);
}

void bicg_solve(gsl_matrix *A, gsl_vector *b, gsl_vector *x, gsl_vector *initial_guess, double itol, int n) {
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
    double rho, rho1, beta, omega, alpha, diagElement;

    gsl_vector_memcpy(x, initial_guess); // x = initial_guess

    gsl_blas_dgemv(CblasNoTrans, -1.0, A, x, 0.0, r);
    gsl_vector_add(r, b); // r = b - Ax
    gsl_vector_memcpy(r_tilde, r); // r_tilde = r

    while (gsl_blas_dnrm2(r) / gsl_blas_dnrm2(b) > itol && iter < n) {
        iter++;

        for (int i = 0; (i < A->size1) || (i < A->size2); ++i) {
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

        gsl_blas_daxpy(alpha, p, x); // x = x + alpha * p
        gsl_blas_daxpy(-alpha, q, r); // r = r - alpha * q
        gsl_blas_daxpy(-alpha, q_tilde, r_tilde); // r_tilde = r_tilde - alpha * q_tilde

        rho1 = rho;
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