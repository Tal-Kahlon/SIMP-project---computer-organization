#include <stdlib.h>
#include <stdio.h>
#include <crtdbg.h>
#define MAX_LABEL 50
#define MAX_LINE 500
#pragma warning(disable:4996);
#define _CRT_SECURE_NO_WARNINGS


typedef struct line //define a struct to store a line from .acm file
{
	char opcode_name[MAX_LABEL + 2];//label can be 50 chars and the ":" and "\0" takes 2 more chars
	char rd[MAX_LINE];
	char rs[MAX_LINE];
	char rt[MAX_LINE];
	char imm[MAX_LINE];
	struct line* next;
	int is_label; //number of line that label appears
	int R_or_I; // I format command or R format
} line;

typedef struct word { //define a struct to store a '.word  from .acm file
	int line_number;
	int value;
	struct word* next;
} word;

// function create_lines_list get pointer of linked list ,new line , line counter 
// and retun the haead of the linked list
// the fuction build the linked list of line 
line* create_lines_list(line* all_lines, char* new_line, int* line_counter) {
	line* new_link = (line*)malloc(sizeof(line));//allocate memory for new link in list
	int count = 0, i = 0;//count itterates over the line, i itterates over the struct fields
	int label = 0;//an indication if the current line is a label
	line* ptr = NULL;//pointer to navigate throw the linked list
	if (new_link == NULL) return NULL;//check allocation
	new_link->is_label = -1;//reset the label flag
	new_link->R_or_I = 0;//reset the format to R 
	int flag = 0;
	//next loop fills the fieldes of the struct it has 5 repetitions for the 5 different fields
	//name field
	while ((new_line[count] != '\n') & (new_line[count] != ' ') & (new_line[count] != ',') & (label == 0)) { //this while func build the opcode_name
		if (new_line[count] == '\t') {
			count++;
			continue;
		}
		else if (new_line[count] == ':') {//the odentifier of a label
			label = 1;
			new_link->is_label = *line_counter;//change the label field to the labels address
			*line_counter = *line_counter - 1;//we do not count labels as lines

		}
		new_link->opcode_name[i] = new_line[count];//copy the char to the right field 
		i++;
		count++;
	}
	new_link->opcode_name[i] = '\0';//end the field content



	//rd field
	i = 0;
	count++;
	while ((new_line[count] != '\n') & (new_line[count] != ',') & (label == 0) & flag != 2) {
		if (new_line[count] == '\t') {
			count++;
			continue;
		}
		if (new_line[count] == ' ') {
			if (flag == 1) {
				flag = 2;
				continue;
			}
			count++;
			continue;

		}
		new_link->rd[i] = new_line[count];
		if (strcmp(new_link->opcode_name, ".word") == 0) flag = 1;

		i++;
		count++;
	}
	new_link->rd[i] = '\0';
	i = 0;
	count++;

	//rs field 

	while ((new_line[count] != '\n') & (new_line[count] != ',') & (label == 0)) {
		if ((new_line[count] == '\t') | (new_line[count] == ' ')) {
			count++;
			continue;
		}
		new_link->rs[i] = new_line[count];
		i++;
		count++;
	}
	new_link->rs[i] = '\0';

	//rt field
	if (strcmp(new_link->opcode_name, ".word") == 0) {//checking if this is the last line
		new_link->imm[0] = '\0';
		new_link->rt[0] = '\0';
	}
	else {
		i = 0;
		count++;
		while ((new_line[count] != '\n') & (new_line[count] != ',') & (label == 0)) {
			if ((new_line[count] == '\t') | (new_line[count] == ' ')) {
				count++;
				continue;
			}
			new_link->rt[i] = new_line[count];
			i++;
			count++;
		}
		new_link->rt[i] = '\0';

		//imm field 
		i = 0;
		count++;
		while ((new_line[count] != '\n') & (new_line[count] != ',') & (new_line[count] != '#') & (label == 0)) {
			if ((new_line[count] == '\t') | (new_line[count] == ' ')) {
				count++;
				continue;
			}
			new_link->imm[i] = new_line[count];
			i++;
			count++;
		}
		new_link->imm[i] = '\0';
	}

	//check the format type and change it only if it's I format 
	if (strcmp(new_link->rd, "$imm") == 0) {
		new_link->R_or_I = 1;
		*line_counter = *line_counter + 1;
	}
	else if (strcmp(new_link->rs, "$imm") == 0) {
		new_link->R_or_I = 1;
		*line_counter = *line_counter + 1;
	}
	else if (strcmp(new_link->rt, "$imm") == 0) {
		new_link->R_or_I = 1;
		*line_counter = *line_counter + 1;
	}

	//adding the new_link to the linked list for the first time
	if (all_lines == NULL) {
		all_lines = new_link;
		all_lines->next = NULL;
		*line_counter = *line_counter + 1;
		return all_lines;
	}
	// adding the new_link to the linked list not for the first time
	else {
		ptr = all_lines;
		while (all_lines->next != NULL) all_lines = all_lines->next;
		all_lines->next = new_link;
		new_link->next = NULL;
		*line_counter = *line_counter + 1;
		return ptr;
	}



}

// function read_line get the asm file and return linked list of lines
// function call to create_lines_list with a new line from asm file to all asm bile
line* read_lines(FILE* asmfile) {
	char* new_line = (char*)malloc(MAX_LINE);//allocate memory for new line
	line* all_lines = NULL;
	int line_counter = 0;//line counter for the labels
	if (new_line == NULL) { printf("error allocating memory"); exit(1); }//chack allocation
	//The next loop reads the file line by line and sends it to "all_lines" that creates 
	//a linked list of all lines devided to parts
	while (((fgets(new_line, 500, asmfile)) != NULL) & (new_line != EOF)) {
		all_lines = create_lines_list(all_lines, new_line, &line_counter);//send to a func. that builds the line linked list

	}
	free(new_line); // free memory of last line that not in use 

	return all_lines;
}

//function get the heat of the linked list of lines and the label
//function return the line number of the label
// function find the line number that label is appears
int find_label_and_change(line* file_lines, char* imm) { /// change label to his line number 
	line* head1 = file_lines;//head of linked list
	char imm1[51];
	int how_many_word = 0;
	strcpy(imm1, imm);
	strcat(imm1, ":");
	while (head1 != NULL) {
		if (strcmp(head1->opcode_name, ".word") == 0) {
			how_many_word++;
		}
		if (strcmp(head1->opcode_name, imm1) == 0) {
			return ((head1->is_label) - how_many_word);
		}
		head1 = head1->next;
	}

}
// function change_labels_to_adrresses get the head of the linmed list of line 
// function change head->imm to the number of line of the label
void change_labels_to_adrresses(line* file_lines) {//need to finish it!
	line* head = file_lines;//head of linked list
	int line_number;
	while (head != NULL) {
		if ((head->imm[0] >= 'A' && head->imm[0] <= 'Z') || (head->imm[0] >= 'a' && head->imm[0] <= 'z')) {
			line_number = find_label_and_change(file_lines, head->imm); // function find_label_and_change return the number line of the label
			itoa(line_number, head->imm, 10);
		}
		head = head->next;
	}
	return;

}
//function get opcode name and memin file and retur the value of the opcode name 
// function check what value is suitable for opcode name and then print it to memin
// if opcode name is .word retuen 0 else 1
int convert_opcode_to_hexa(char* opcode_name, FILE* memin) { // covert opcode to hexa and print 
	if (strcmp(opcode_name, "add") == 0) {
		fprintf(memin, "00"); return 1;
	}
	else if (strcmp(opcode_name, "sub") == 0) {
		fprintf(memin, "01"); return 1;
	}
	else if (strcmp(opcode_name, "mul") == 0) {
		fprintf(memin, "02"); return 1;
	}
	else if (strcmp(opcode_name, "and") == 0) {
		fprintf(memin, "03"); return 1;
	}
	else if (strcmp(opcode_name, "or") == 0) {
		fprintf(memin, "04"); return 1;
	}
	else if (strcmp(opcode_name, "xor") == 0) {
		fprintf(memin, "05"); return 1;
	}
	else if (strcmp(opcode_name, "sll") == 0) {
		fprintf(memin, "06"); return 1;
	}
	else if (strcmp(opcode_name, "sra") == 0) {
		fprintf(memin, "07"); return 1;
	}
	else if (strcmp(opcode_name, "srl") == 0) {
		fprintf(memin, "08"); return 1;
	}
	else if (strcmp(opcode_name, "beq") == 0) {
		fprintf(memin, "09"); return 1;
	}
	else if (strcmp(opcode_name, "bne") == 0) {
		fprintf(memin, "0A"); return 1;
	}
	else if (strcmp(opcode_name, "blt") == 0) {
		fprintf(memin, "0B"); return 1;
	}
	else if (strcmp(opcode_name, "bgt") == 0) {
		fprintf(memin, "0C"); return 1;
	}
	else if (strcmp(opcode_name, "ble") == 0) {
		fprintf(memin, "0D"); return 1;
	}
	else if (strcmp(opcode_name, "bge") == 0) {
		fprintf(memin, "0E"); return 1;
	}
	else if (strcmp(opcode_name, "jal") == 0) {
		fprintf(memin, "0F"); return 1;
	}
	else if (strcmp(opcode_name, "lw") == 0) {
		fprintf(memin, "10"); return 1;
	}
	else if (strcmp(opcode_name, "sw") == 0) {
		fprintf(memin, "11"); return 1;
	}
	else if (strcmp(opcode_name, "reti") == 0) {
		fprintf(memin, "12"); return 1;
	}
	else if (strcmp(opcode_name, "in") == 0) {
		fprintf(memin, "13"); return 1;
	}
	else if (strcmp(opcode_name, "out") == 0) {
		fprintf(memin, "14"); return 1;
	}
	else if (strcmp(opcode_name, "halt") == 0) {
		fprintf(memin, "15"); return 1;
	}
	return 0;
}

// function get variable (rd,rs,rt) and memin file
// function check what value is suitable for variable and then print it to memin
void print_var(char* var, FILE* memin) { // convert rt,rs,rd to hexa
	if (strcmp(var, "$zero") == 0)
		fprintf(memin, "0");
	else if (strcmp(var, "$imm") == 0)
		fprintf(memin, "1");
	else if (strcmp(var, "$v0") == 0)
		fprintf(memin, "2");
	else if (strcmp(var, "$a0") == 0)
		fprintf(memin, "3");
	else if (strcmp(var, "$a1") == 0)
		fprintf(memin, "4");
	else if (strcmp(var, "$a2") == 0)
		fprintf(memin, "5");
	else if (strcmp(var, "$a3") == 0)
		fprintf(memin, "6");
	else if (strcmp(var, "$t0") == 0)
		fprintf(memin, "7");
	else if (strcmp(var, "$t1") == 0)
		fprintf(memin, "8");
	else if (strcmp(var, "$t2") == 0)
		fprintf(memin, "9");
	else if (strcmp(var, "$s0") == 0)
		fprintf(memin, "A");
	else if (strcmp(var, "$s1") == 0)
		fprintf(memin, "B");
	else if (strcmp(var, "$s2") == 0)
		fprintf(memin, "C");
	else if (strcmp(var, "$gp") == 0)
		fprintf(memin, "D");
	else if (strcmp(var, "$sp") == 0)
		fprintf(memin, "E");
	else if (strcmp(var, "$ra") == 0)
		fprintf(memin, "F");
}

//function get hex_imm, string imm anf memin file 
// function print imm to memin in case imm is hex
void change_to_right_format_and_print(char* hex_imm, char* imm, FILE* memin) {
	int len = strlen(imm);
	int j = 0;
	for (int i = 2; i < len; i++) {
		hex_imm[i - 2] = imm[i];//copy all he chars after the second char
		j = i;
	}
	hex_imm[j - 1] = '\0';//finish the string
	len = strlen(hex_imm);
	if (len == 5)
		fprintf(memin, "%s", hex_imm);
	else if (len == 4)
		fprintf(memin, "0%s", hex_imm);
	else if (len == 3)
		fprintf(memin, "00%s", hex_imm);
	else if (len == 2)
		fprintf(memin, "000%s", hex_imm);
	else if (len == 1)
		fprintf(memin, "0000%s", hex_imm);
}

//function get imm and memin file 
// function print imm to memin file and make sign extension
void printimm(char* imm, FILE* memin) {
	if ((imm[0] == '0' && imm[1] == 'x') | (imm[0] == '0' && imm[1] == 'X')) { // Checks if exhadmili number  
		char hex_imm[52];//uses to store the hex input
		change_to_right_format_and_print(hex_imm, imm, memin); // function print to memin if imm is hex
	}
	else {
		int num = atoi(imm);
		if (num >= 0) //Checks if a positive number
			fprintf(memin, "%05X", num);
		if (num < 0) { // Checks for a negative number
			num = abs(num);
			num = (~(num - 1));
			fprintf(memin, "%05X", num & 0xfffff);
		}
	}

}
//function get linked list of word and current line 
//return 1 if there word should be wriiten to current line else 0 
//function check if there is a word that should be written in current line 
int checks_if_exists(word* word_line, int line_counter) {
	int exists = 0;
	word* head_word = word_line;
	while (head_word != NULL && exists == 0) {
		if (head_word->line_number == line_counter) {
			exists = 1;
		}
		head_word = head_word->next;
	}
	return exists;
}

//function get linked list of word and current line 
//return the higest line number that word exists
// function find the higest line number that word exists
int find_higest_line_word(word* word_line, int line_counter) {
	int higest_value_word = 0;
	word* head_word = word_line;
	while (head_word != NULL) {
		if (head_word->line_number > higest_value_word) {
			higest_value_word = head_word->line_number;
		}
		head_word = head_word->next;
	}
	return higest_value_word;
}

//function get linked list of word and current line 
//return value of word that should be written 
//function find relevant value of the relevant word considre by current line
int find_value_word(word* word_line, int line_counter) {
	int value = 0;
	word* head_word = word_line;
	while (head_word != NULL && value == 0) {
		if (head_word->line_number == line_counter) {
			value = head_word->value;
		}
		head_word = head_word->next;
	}
	return value;
}

// function get linked list of lines, linked loist of word and memin file
//function print to memin as required
void print_to_memin(line* file_lines, word* word_line, FILE* memin) {
	int line_counter = 0;//in order to add zeroes at the end
	int there_is_word = 0;
	int higest_line_word = 0;
	int value_word = 0;
	line* head = file_lines;//head of the lines list
	word* head_word = word_line;
	int if_word = 0;
	while (head != NULL) {
		if (head->is_label == -1) {   //not label
			there_is_word = checks_if_exists(word_line, line_counter); //check if word existes in current line (1 = if exists)
			if (there_is_word == 1) { //if exists =1 
				value_word = find_value_word(word_line, line_counter); // find value for relevante word
				fprintf(memin, "%05X\n", value_word);
				line_counter++;
			}
			else {
				if_word = convert_opcode_to_hexa(head->opcode_name, memin); //check if not word function return 0 if opcode name is word else return 1
				if (if_word != 0) {
					print_var(head->rd, memin); // print rd to memin 
					print_var(head->rs, memin); // print rs to memin 
					print_var(head->rt, memin); // print rt to memin 
					fprintf(memin, "\n");
					line_counter = line_counter + 1;
				}
			}
			if (head->R_or_I == 1) {// i format. there is imm 
				there_is_word = checks_if_exists(word_line, line_counter);//check if word existes in current line (1 = if exists)
				if (there_is_word == 1) { // if word 
					value_word = find_value_word(word_line, line_counter);// find relvante value for word
					fprintf(memin, "%05X", value_word);
				}
				else {
					printimm(head->imm, memin); // print imm to memin with sign extension
				}
				fprintf(memin, "\n");
				line_counter = line_counter + 1;
			}
		}
		head = head->next;

	}
	higest_line_word = find_higest_line_word(word_line, line_counter); //if left lines between the end of ths file till word line
	for (int i = line_counter; i <= higest_line_word; i++) {
		there_is_word = checks_if_exists(word_line, i); //check if word existes in current line (1 = if exists)
		if (there_is_word == 1) {
			value_word = find_value_word(word_line, i); // find relvante value for word
			if (value_word >= 0)
				fprintf(memin, "%05X\n", value_word);
			else {
				value_word = abs(value_word);
				value_word = (~(value_word - 1));
				fprintf(memin, "%05X", value_word & 0xfffff);
			}
		}
		else fprintf(memin, "00000\n");
	}
}

// function get hex char number with start of 0x and return number as int 
//function convert hex number to decimal number
int convert_from_hex_to_dec(char hex[]) {
	int length = 0, decimal = 0, base = 1;
	int end = 0;
	char hex1[6];
	int len_hex = strlen(hex);
	for (int j = 0; j < len_hex - 2; j++) {
		hex1[j] = hex[j + 2];
		end = j;
	}
	hex1[end + 1] = '\0';
	length = strlen(hex1);
	for (int i = length--; i >= 0; i--)
	{
		if (hex1[i] >= '0' && hex1[i] <= '9')
		{
			decimal += (hex1[i] - 48) * base;
			base *= 16;
		}
		else if (hex1[i] >= 'A' && hex1[i] <= 'F')
		{
			decimal += (hex1[i] - 55) * base;
			base *= 16;
		}
		else if (hex1[i] >= 'a' && hex1[i] <= 'f')
		{
			decimal += (hex1[i] - 87) * base;
			base *= 16;
		}
	}
	return decimal;
}
// function get the head of the linked list of lines and return the head of the linked list of word
// function find .word in linked line and make a new linked list of .word
word* word_data(line* file_lines) {
	word* head_new_word = NULL;
	word* temp = NULL;
	line* head = file_lines;//pointer to navigate throw the linked list
	while (head != NULL) {
		if (strcmp(head->opcode_name, ".word") == 0) {
			word* new_word = (word*)malloc(sizeof(word));
			if (new_word == NULL) return NULL;
			//take care of hex numbers

			if (((head->rd[0] == '0') && (head->rd[1] == 'x')) || ((head->rd[0] == '0') && (head->rd[1] == 'X'))) // if it start with haxa 
				new_word->line_number = convert_from_hex_to_dec(head->rd); // function convert_from_hex_to_dec return the number that appears in decimal
			else
				new_word->line_number = atoi(head->rd);
			if (((head->rs[0] == '0') && (head->rs[1] == 'x')) || ((head->rs[0] == '0') && (head->rs[1] == 'X')))// if it start with haxa 
				new_word->value = convert_from_hex_to_dec(head->rs);  // function convert_from_hex_to_dec return the number that appears in decimal
			else
				new_word->value = atoi(head->rs);
			// adding new_word to the linked list of word
			if (temp == NULL) {
				temp = new_word;
				head_new_word = temp;
				temp->next = NULL;
			}
			else {
				temp->next = new_word;
				temp = temp->next;
				temp->next = NULL;
			}
		}

		head = head->next;
	}
	return head_new_word;
}


//function fet linked list of lines and linked list of word 
// free all memory from both linked list
void free_all_allocated_mem(line* all_lines, word* all_words) {
	line* curr_line = NULL;
	word* curr_word = NULL;
	while (all_lines != NULL) {
		curr_line = all_lines;
		all_lines = all_lines->next;
		free(curr_line);
	}
	free(all_lines);
	while (all_words != NULL) {
		curr_word = all_words;
		all_words = all_words->next;
		free(curr_word);
	}
	free(all_words);

	return;
}

main(int argc, char* argv[]) {
	FILE* asmfile = NULL;//Define the file pointer 
	asmfile = fopen(argv[1], "r");//open the .asm file
	if (asmfile == NULL) exit(1);//If opening failed exit
	line* file_lines = read_lines(asmfile);//function return linked list of lines from asm file
	fclose(asmfile);
	change_labels_to_adrresses(file_lines);// change label to adrresses in linked list of lines
	word* word_lines = word_data(file_lines); //function return linked list of word
	FILE* memin = fopen(argv[2], "w"); // open memin file
	if (memin == NULL)
		exit(1);
	print_to_memin(file_lines, word_lines, memin); // function print to memin file 
	fclose(memin);
	free_all_allocated_mem(file_lines, word_lines); 	//function that deletes all allocated memory
	return 0;
}
