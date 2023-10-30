#include "parser.h"

void parser(FILE *input_file, Element **head, NodePair **head_node_pair) {
	char *line = NULL;
	char **line_array;
	unsigned long pair=0;
	size_t len;
	ssize_t read;
	Element *current=NULL;
	int ret_val=-1;

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

	while(1) { // Change implementation to be a bit more generic
		// Distinguish between different compontents and make the list


		for (int i=0;i<NUM_OF_ELEMENT_DATA;i++) {
			strncpy(line_array[i], "\0", MAX_CHAR_NUM);
		}

		ret_val = fscanf(input_file, "%s %s %s %s\n", line_array[0], line_array[1], line_array[2], line_array[3]);

//		if (line[strlen(line)-1] != '\n') {
//			// Padding the last line
//			line = realloc(line, (strlen(line)+1)*sizeof(char));
//			line[strlen(line)] = '\n';
//		}

		if ((ret_val == 0) || (ret_val == EOF)) {
			break;
		}
		
		if (line_array[0][0] == '*') {
			// This fscanf skips after a new line character
			fscanf(input_file, SKIP_NEWLINE);
			continue;
		}

		strToLower(line_array[0]);
		strToLower(line_array[1]);
		strToLower(line_array[2]);
		strToLower(line_array[3]);


//		if (remove_spaces(line, line_array) == 0) {
//			print_error("parser", 4, "Remove spaces in line failed");
//		}

		if (current != NULL) { // Find the next available free list
			while (current->next != NULL) {
				current = current->next;
			}
		}

		if ((line_array[0][0] != '*') && (line_array[0][0] != '.')) { // Ignore comments that start with *
			// Copy every element's value such as type, name, nodes etc
			current->type_of_element = line_array[0];
			current->name = strdup(&line_array[0][1]);

			current->node_p = strdup(line_array[1]); // Needs to be free'd
			current->node_n = strdup(line_array[2]);

			// Calculate the hash and add it to the db, if not found
			pair = find_node_pair(*head_node_pair, current->node_p);
			if (pair == -1) {
				add_node_pair(head_node_pair, hash(line_array[1]), line_array[1]);
			}
			pair = find_node_pair(*head_node_pair, current->node_n);
			if (pair == -1) {

				add_node_pair(head_node_pair, hash(line_array[2]), line_array[2]);
			}
			current->value = strtod(line_array[3], NULL);
		}
		else if (line[0] == '.') {
			// Has to be implemented
			// get_analysis_type;
			fscanf(input_file, SKIP_NEWLINE);
		}
		
		// Go to the next element
		current->next = calloc(1, sizeof(Element));
		current->next->prev = current;
	}
	free(line);
	free_mem(line_array, NULL, NULL);
}

void get_analysis_type(char* line, AnalysisType **type_struct) {
	AnalysisType *current = NULL;

	if (!*type_struct) {
		*type_struct = calloc(1, sizeof(AnalysisType));
	}
	
	current = *type_struct;
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

void print_error (char* program_name ,int error_code, char* comment) {
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