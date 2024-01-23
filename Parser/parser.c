#include "parser.h"

void parser(FILE *input_file, Element **head, NodePair **head_node_pair, RetHelper *ret, SpiceAnalysis *options) {
	char *line = NULL;
	char **line_array;
	unsigned long pair=0;
	Element *current=NULL;
	int ret_val=-1;
	//counts the amount of elements that belong to group2
	unsigned int group2_el = 0;			
	unsigned long elements_read = 0;
	int total_allocs = 0;
	char plot_line_buffer[MAX_LINE_BUFF_LEN]={'\0'};
	int true_size = STARTING_ARR_NUM;
	char temp_element[MAX_CHAR_NUM]={'\0'};
	char *testline = NULL, *temptestline = NULL;
	EXP_T *exp_data = NULL;
	SIN_T *sin_data = NULL;
	PULSE_T *pulse_data = NULL;
	PWL_T *pwl_data = NULL;

	// Create line split 2D array
	line_array = calloc(NUM_OF_ELEMENT_DATA, sizeof(char*));
	if (!line_array) {
		print_error("parser", 3, "");
	}
	for (int i=0;i<NUM_OF_ELEMENT_DATA;i++) {
		line_array[i] = calloc(MAX_CHAR_NUM, sizeof(char));
	}

	// Head for element linked list
	if (!*head) {
		*head = calloc(1, sizeof(Element));	
	}
	current = *head;

	// Head for pairs linked list
	if (!*head_node_pair) {
		*head_node_pair = calloc(1, sizeof(NodePair));
	}

	if (!options) {
		options = calloc(1, sizeof(SpiceAnalysis));
	}

	while(1) { // Change implementation to be a bit more generic
		// Distinguish between different compontents and make the list


		for (int i=0;i<NUM_OF_ELEMENT_DATA;i++) {
			strncpy(line_array[i], "\0", MAX_CHAR_NUM);
		}

		// Read the first string and determine if it's an element or some command
		ret_val = fscanf(input_file, "%s", line_array[0]);

		if ((ret_val == 0) || (ret_val == EOF)) {
			break;
		}
		
		if (line_array[0][0] == '*') {
			// This fscanf skips after a new line character
			fscanf(input_file, SKIP_NEWLINE);
			continue;
		}

		strToLower(line_array[0]);

		if (!strcmp(line_array[0], SPICE_END)) {
			break;
		}

		if (current != NULL) { // Find the next available free list
			while (current->next != NULL) {
				current = current->next;
			}
		}

		if (line_array[0][0] != '.') { // Ignore comments that start with *
			// Copy every element's value such as type, name, nodes etc
			fscanf(input_file, "%s %s %s%[^\n]\n", line_array[1], line_array[2], line_array[3],line_array[4]);
			for (int i=1;i<NUM_OF_ELEMENT_DATA;i++) {
				strToLower(line_array[i]);
			}
			
			current->type_of_element = line_array[0][0];
			current->name = strdup(&line_array[0][1]);

			current->node_p = strdup(line_array[1]); // Needs to be free'd
			current->node_n = strdup(line_array[2]);
			current->group_flag = 0;	//set the element's group flag to 0 proactively and check immediately after if it needs to change. 
										//If it changes increment by 1 the amount of elements in group2
			elements_read++;
			if(current->type_of_element == 'v' || current->type_of_element == 'l'){
				if ((strcmp(line_array[1], "0") != 0) && (strcmp(line_array[2], "0") != 0)) {
					ret->non_zero_elements += 4;
				}
				else {
					ret->non_zero_elements += 2;
				}
				group2_el++;
				current->group_flag = 1;	
			}
			if (current->type_of_element == 'r') {
				if ((strcmp(line_array[1], "0") != 0) && (strcmp(line_array[2], "0") != 0)) {
					ret->non_zero_elements += 4;
				}
				else {
					ret->non_zero_elements += 2;
				}
			}
			// Calculate the hash and add it to the db, if not found
			pair = find_node_pair(*head_node_pair, current->node_p);
			if (pair == -1) {
				add_node_pair(head_node_pair, strcmp(line_array[1], "0")==0?0:++ret->amount_of_nodes, line_array[1]);
				//ret->amount_of_nodes++;
			}
			pair = find_node_pair(*head_node_pair, current->node_n);
			if (pair == -1) {

				add_node_pair(head_node_pair, strcmp(line_array[2], "0")==0?0:++ret->amount_of_nodes, line_array[2]);
				//ret->amount_of_nodes++;
			}
			current->value = strtod(line_array[3], NULL);
			// Go to the next element
			current->next = calloc(1, sizeof(Element));
			current->next->prev = current;

			//time to parse the transient part if it exists
			if(line_array[4] != NULL){
				if((temptestline = strstr(line_array[4],"exp"))!=NULL){
					current->tran_data_type=1;
					testline=temptestline;
				}
				if((temptestline = strstr(line_array[4],"sin"))!=NULL){
					current->tran_data_type=2;
					testline=temptestline;
				}
				if((temptestline = strstr(line_array[4],"pulse"))!=NULL){
					current->tran_data_type=3;
					testline=temptestline;
				}
				if((temptestline = strstr(line_array[4],"pwl"))!=NULL){
					current->tran_data_type=4;
					testline=temptestline;
				}
				if(testline == NULL){
					testline = NULL;
					current->tran_data_type = 0;
					fprintf(stderr, "\nWrong format for transient analysis\n");
					exit(1);
				}
				switch(current->tran_data_type){
					case 1:
						{
							current->transient_type_info = malloc(sizeof(EXP_T));
							if(current->transient_type_info == NULL){
        						fprintf(stderr, "Memory allocation failed\n");
        						exit(1);
    						}
							exp_data = (EXP_T*) current->transient_type_info;
							int res = sscanf(testline, "exp (%lf %lf %lf %lf %lf %lf)",&exp_data->i1, &exp_data->i2, &exp_data->td1, &exp_data->tc1, &exp_data->td2, &exp_data->tc2);
							if(res != 6){
        						fprintf(stderr, "Parsing failed\n");
        						// Handle parsing failure as needed (e.g., free allocated memory)
        						free(exp_data);
        						exit(1);
							}	
							break;
						}
					case 2:
						{
							current->transient_type_info = malloc(sizeof(SIN_T));
							if(current->transient_type_info == NULL){
        						fprintf(stderr, "Memory allocation failed\n");
        						exit(1);
    						}
							sin_data = (SIN_T*) current->transient_type_info;
							int res = sscanf(testline, "sin (%lf %lf %lf %lf %lf %lf)",&sin_data->i1, &sin_data->ia, &sin_data->fr, &sin_data->td, &sin_data->df, &sin_data->ph);
							if(res != 6){
        						fprintf(stderr, "Parsing failed\n");
        						// Handle parsing failure as needed (e.g., free allocated memory)
        						free(sin_data);
        						exit(1);
							}	
							break;
						}
					case 3:
						{
							current->transient_type_info = malloc(sizeof(PULSE_T));
							if(current->transient_type_info == NULL){
        						fprintf(stderr, "Memory allocation failed\n");
        						exit(1);
    						}
							pulse_data = (PULSE_T*) current->transient_type_info;
							int res = sscanf(testline, "pulse (%lf %lf %lf %lf %lf %lf %lf)",&pulse_data->i1, &pulse_data->i2, &pulse_data->td, &pulse_data->tr, &pulse_data->tf, &pulse_data->pw, &pulse_data->per);
							if(res != 7){
        						fprintf(stderr, "Parsing failed\n");
        						// Handle parsing failure as needed (e.g., free allocated memory)
        						free(pulse_data);
        						exit(1);
							}	
							break;	
						}
					case 4:
						{
							char *stringcheck = testline;
							int tuples_counter = 0;

							while(*stringcheck){
								if(*stringcheck == '('){
									tuples_counter++;
								}
								stringcheck++;
							}

							current->transient_type_info = malloc(sizeof(PWL_T)*tuples_counter);
							if(current->transient_type_info == NULL){
        						fprintf(stderr, "Memory allocation failed\n");
        						exit(1);
    						}
							pwl_data = (PWL_T*) current->transient_type_info;
							stringcheck = testline;
							for(int i=0; i < tuples_counter; i++){
								int res = sscanf(stringcheck, "(%lf %lf)",&pwl_data[i].val1, &pwl_data[i].val2);
								if (res != 2) {
									fprintf(stderr, "Parsing failed\n");
									// Handle parsing failure as needed (e.g., free allocated memory)
									free(current->transient_type_info);
									exit(1);
								}
								while (*stringcheck && (*stringcheck != '(')) {
										stringcheck++;
								}
							}
							break;	
						}				
				}
			}
			else{
				current->tran_data_type = 0;
			}
		}
		else {
			if (strcmp(line_array[0], OPTIONS) == 0) { // .OPTIONS statements
				fscanf(input_file, "%s", line_array[1]);
				strToLower(line_array[1]);
				if (strcmp(line_array[1], CHOLESKY_OPTION) == 0) {
					ret->direct_chol_flag = 1;
					fprintf(stderr, "\nCholesky decomposition is used\n");
				}
				else if (strcmp(line_array[1], USE_ITERATIONS_OPTION)==0) { // Check if iterations are used
					char next_char = fgetc(input_file);
					if ((next_char == '\n') || (next_char == '\r')){ // If new line, skip to new line
						ret->use_iterations = 1;
						fprintf(stderr, "\nIteration is used (Bi-CG)\n");
					}
					else { // Else, read other options
						fscanf(input_file, "%s", line_array[2]);
						strToLower(line_array[2]);
						if (strcmp(line_array[2], CHOLESKY_OPTION) == 0) {
							ret->use_iterations_cg = 1;
							fprintf(stderr, "\nIteration is used (CG)\n");
						}
					}
				}
				else if (strncmp(line_array[1], SPARSE_OPTION, strlen(SPARSE_OPTION)) == 0) {
					ret->sparse = true;

				}
				else if (strncmp(line_array[1], GET_TOLERANCE, strlen(GET_TOLERANCE)) == 0) { // If iterations are used, check if the user gave a new tolerance
					if (ret->use_iterations || ret->use_iterations_cg) {
						sscanf(&line_array[1][strlen(GET_TOLERANCE)], "%lf", &ret->tolerance);
						// fprintf(stderr, "\nNew tolerance is %lf\n", ret.max_iter_num);
					}
					else {
						fprintf(stderr, "\nTolerance is not used, as iterations are not used\n");
					}
				}
			}
			else if (strcmp(line_array[0], DC_ANALYSIS) == 0) { // .OP Analysis
				if ((options->DC_OP == false) && (!options->DC_SWEEP)) {
					options->DC_OP = true;
					fprintf(stderr, "\n\n.OP Selected\n\n"); // Debug only
				}
				else {
					// Debug only
					fprintf(stderr, "\n\nOP method not selected, as there is another one already\n\n");
				}
			}
			else if (strcmp(line_array[0], DC_SWEEP) == 0) { // .DC Sweep Analysis
				for (int i=0;i<NUM_OF_ELEMENT_DATA;i++) {
					strncpy(line_array[i], "\0", MAX_CHAR_NUM);
				}
				
				if ((options->DC_OP == true) || (options->DC_SWEEP)) {
					fprintf(stderr, "\n\nDC Sweep method not selected, as there is another one already\n\n");
					continue;
				}
				options->DC_OP = false;
				fprintf(stderr, "\n\nDC Sweep Selected\n\n"); // Debug only

				fscanf(input_file, "%s %s %s %s\n", line_array[1], line_array[2], line_array[3], line_array[4]);
				
				for (int i=0;i<NUM_OF_ELEMENT_DATA;i++) {
					strToLower(line_array[i]);
				}

				options->DC_SWEEP = calloc(1, sizeof(struct dc_sweep_opts));
				if (!options->DC_SWEEP) {
					print_error("parser", 3, "Error while creating DC sweep struct");
				}

				switch (line_array[1][0]) {
					case 'v': {
						options->DC_SWEEP->is_voltage = true;
						break;
					}
					case 'i': {
						options->DC_SWEEP->is_voltage = false;
						break;
					}
					default: {
						print_error("parser", 3, "DC sweep was not a SOURCE");
						break;
					}
				}
				options->DC_SWEEP->variable_type = line_array[1][0];
				options->DC_SWEEP->variable_name = strdup(&line_array[1][1]);
				options->DC_SWEEP->start_val = strtod(line_array[2], NULL);
				options->DC_SWEEP->end_val = strtod(line_array[3], NULL);
				options->DC_SWEEP->increment = strtod(line_array[4], NULL);
			}
			else if ((strcmp(line_array[0], PLOT) == 0) || (strcmp(line_array[0], PRINT) == 0)) {

				for (int i=0;i<MAX_LINE_BUFF_LEN;i++) {
					plot_line_buffer[i] = '\0';
				}

				fgets(plot_line_buffer, MAX_LINE_BUFF_LEN, input_file);

				options->PLOT = calloc(1, sizeof(struct plot_opts));
				if (!options->PLOT) {
					print_error("parser", 3, "Error while creating PLOT options struct");
				}

				options->PLOT->elements_to_print = calloc(true_size, sizeof(char*));
				for (int i=0;i<true_size;i++) {
					options->PLOT->elements_to_print[i] = calloc(MAX_CHAR_NUM, sizeof(char));
				}

				for (int i=0;i<strlen(plot_line_buffer);) {
					// Get each plot agrument from the buffer line

					for (int j=0;j<MAX_CHAR_NUM;j++) {
						temp_element[j] = '\0';
					}

					if (sscanf(&plot_line_buffer[i], "%s ", temp_element) == -1) {
						// Exit when it reads new line or EOF
						break;
					}
					i += strlen(temp_element)+1; // Go to the next element
					total_allocs++; // Increase total allocations number
					if (true_size - total_allocs == 0) { // Realloc the array if full
						true_size += STARTING_ARR_NUM;
						options->PLOT->elements_to_print = realloc(options->PLOT->elements_to_print, true_size*sizeof(char *));
						if (!options->PLOT->elements_to_print) {
							print_error("parser", 3, "Reallocating memory for plot elements to print failed!");
						}
						for (int j=total_allocs;j<true_size;j++) {
							options->PLOT->elements_to_print[j] = calloc(MAX_CHAR_NUM, sizeof(char));
							if (!options->PLOT->elements_to_print[j]) {
								print_error("parser", 3, "Allocating new mem for plot elements failed!");
							}
						}
					}
					strncpy(options->PLOT->elements_to_print[total_allocs-1], temp_element, strlen(temp_element));
				}

				// Free the remaining lines
				for (int i=total_allocs;i<true_size;i++) {
					free(options->PLOT->elements_to_print[i]);
				}

				options->PLOT->str_num = total_allocs;
			}
		}
	}
	free(line);
	free_mem(line_array, NULL, NULL);
	ret->group2_size = group2_el;
	ret->el_total_size = elements_read;
}


int find_node_pair(NodePair *head, char* node_str) {
	NodePair *current = head;

	while (current != NULL) {
		if (!current->node_str) {
			current = current->next;
			continue;
		}
		if (strcmp(node_str, current->node_str) == 0) {
			return current->hash_node_num;
		}
		current = current->next;
	}
	return -1;
}

int add_node_pair(NodePair **head, unsigned long hash_num, char* node_str) {
	NodePair *current = *head;

	while (current->next != NULL) {
		current = current->next;
	}
	current->next = calloc(1, sizeof(NodePair));
	current = current->next;
	if (!current) {
		print_error("parser", 3, "Add node pair failed!");
		return 0;
	}
	current->hash_node_num = hash_num;
	current->node_str = strdup(node_str);
	return 1;
}

int remove_spaces(char *line, char **line_array) {
	int i=0, start=0, end, j=0;
	
	while (i<strlen(line)) {
		if ((line[i] == ' ') || (line[i] == '\n') || (line[i] == '\t')) {
			if (i+1 < strlen(line)) {
				if (line[i+1] == ' ') {
					i++;
					continue;
				}
			}
			end = i-1;
			line_array[j] = strncpy(line_array[j], &line[start], end-start+1);
			j++;
			start = i+1;
		}
		if (j>=4) {
			return 1;
		}
		i++;
	}
	return 0;
}

unsigned long hash(char * str) {
	unsigned long hash = 5381;
	int c;

	while ((c = *str++)) {
		hash = ((hash << 5) + hash) + c;
	}
	return hash;
}

void strToLower(char *str) {
	for (int i=0;i<strlen(str);i++){
		str[i] = tolower(str[i]);
	}
}

void free_mem (char** lines, Element *head, NodePair *head_pair) {
	Element *current = head, *next = NULL;
	NodePair *currentP = head_pair, *nextP = NULL;

	if (lines) {
		for(int k=0;k<NUM_OF_ELEMENT_DATA;k++) {
			free(lines[k]);
		}
		free(lines);
	}

	while (current != NULL){
		next = current->next;
		free(current->node_n);
		free(current->node_p);
		free(current->name);
		free(current);
		current = next;
	}

	while (currentP != NULL){
		nextP = currentP->next;
		free(currentP->node_str);
		free(currentP);
		currentP = nextP;
	}

}

void print_pairs(NodePair *head) {
	NodePair *current=head;

	while(current!=NULL) {
		if (!current->node_str) {
			current = current->next;
			continue;
		}
		printf("String: %s\tHashed number: %ld\n", current->node_str, current->hash_node_num);
		current = current->next;
	}
}

void print_list(Element *head) {
	Element *current=head;

	while ((current != NULL) && (current->next != NULL)) {
		printf("Element: %c\n", current->type_of_element);
		printf("Name: %s\n", current->name);
		printf("Positive Node: %s\n", current->node_p);
		printf("Negative Node: %s\n", current->node_n);
		printf("Value: %lf\n", current->value);
		printf("Next node: %p\n", current->next);
		printf("Prev node: %p\n\n", current->prev);
		current = current->next;
	}
}

void print_error (char* program_name ,int error_code, const char* comment) {
	switch (error_code) {
		case 1: { // No input file selected
			fprintf(stderr, "\n%s:\terror %d:\tNo input file selected!\n", program_name, error_code);
			exit(-1);
			break;
		}
		case 2: { // No file read
			fprintf(stderr, "\n%s:\terror %d:\tInput file couldn't be opened!\n", program_name, error_code);
			exit(-2);
			break;
		}
		case 3: { // No memory
			fprintf(stderr, "\n%s:\terror %d:\tNot enough memory!\n", program_name, error_code);
			fprintf(stderr, "\t\tComment: %s\n", comment);
			exit(-3);
			break;
		}
		case 4: {
			fprintf(stderr, "\n%s:\terror %d:\tGeneric error!\n", program_name, error_code);
			fprintf(stderr, "\t\tComment: %s\n", comment);
			exit(-4);
			break;
		}
		default: {
			break;
		}
	}
}
