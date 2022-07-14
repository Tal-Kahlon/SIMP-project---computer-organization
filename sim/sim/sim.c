#include <stdio.h>
#include <stdlib.h>
#define REG_SIZE 6
#define MAX_LABEL 50
#define MAX_LINE 500
#pragma warning(disable:4996);

//next function is called from change_registers_after_print
//it is used to print to hwregtrace file
//the inputs are - current number of cycles, read or write int - 0=read, 1=write
//the io register number, the data , file pointers: hwregtrace  leds and display7seg
void print_to_hwregtrace_leds_display7seg_and_update_monitor(int* cycles, int read_write, int reg_number, int content, FILE* hwregtrace, FILE* leds, FILE* display7seg, int* monitor_array, int* io_registers) {
	fprintf(hwregtrace, "%d ", cycles);
	if (read_write == 0)
		fprintf(hwregtrace, "READ ");
	else
		fprintf(hwregtrace, "WRITE ");
	if (reg_number == 0)
		fprintf(hwregtrace, "irq0enable ");
	else if (reg_number == 1)
		fprintf(hwregtrace, "irq1enable ");
	else if (reg_number == 2)
		fprintf(hwregtrace, "irq2enable ");
	else if (reg_number == 3)
		fprintf(hwregtrace, "irq0status ");
	else if (reg_number == 4)
		fprintf(hwregtrace, "irq1status ");
	else if (reg_number == 5)
		fprintf(hwregtrace, "irq2status ");
	else if (reg_number == 6)
		fprintf(hwregtrace, "irqhandler ");
	else if (reg_number == 7)
		fprintf(hwregtrace, "irqreturn ");
	else if (reg_number == 8)
		fprintf(hwregtrace, "clks ");
	else if (reg_number == 9) {
		fprintf(hwregtrace, "leds ");
		if (read_write == 1)
			fprintf(leds, "%d %08X\n", cycles, content);
	}
	else if (reg_number == 10) {
		fprintf(hwregtrace, "display7seg ");
		if (read_write == 1)
			fprintf(display7seg, "%d %08X\n", cycles, content);
	}
	else if (reg_number == 11)
		fprintf(hwregtrace, "timerenable ");
	else if (reg_number == 12)
		fprintf(hwregtrace, "timercurrent ");
	else if (reg_number == 13)
		fprintf(hwregtrace, "timermax ");
	else if (reg_number == 14)
		fprintf(hwregtrace, "diskcmd ");
	else if (reg_number == 15)
		fprintf(hwregtrace, "disksector ");
	else if (reg_number == 16)
		fprintf(hwregtrace, "diskbuffer ");
	else if (reg_number == 17)
		fprintf(hwregtrace, "diskstatus ");
	else if (reg_number == 18 || reg_number == 19)
		fprintf(hwregtrace, "reserved ");
	else if (reg_number == 20)
		fprintf(hwregtrace, "monitoraddr ");
	else if (reg_number == 21)
		fprintf(hwregtrace, "monitordata ");
	else if (reg_number == 22) {
		fprintf(hwregtrace, "monitorcmd ");
		if (read_write == 1) { // change value of pixel , check if write
			if (io_registers[22] == 1) {
				monitor_array[io_registers[20]] = io_registers[21]; //change value(monitordata)
				//of pixel in position (monitoradder)
			}
		}
	}
	fprintf(hwregtrace, "%08X\n", content);
	return;

}

//changes the content of the registers after the print 
//the inputs are - registers (regular and io), pc number, memin array, cycle counterm, monitor array, disk array files: hwregtrace,leds,display7seg
//the function updates the registers according to the instruction, and print to the input files 
int change_registers_after_print(int* registers, int pc, int* io_registers, int line, int* memin, int* cycles, FILE* hwregtrace, FILE* leds, FILE* display7seg, int* monitor_array, int* disk, int* cycle_counter) {
	int op_code = ((line & 0x000ff000) >> 12);
	int rd = ((line & 0x00000f00) >> 8);
	int rs = ((line & 0x000000f0) >> 4);
	int rt = ((line & 0x0000000f));
	int imm = registers[1];

	*cycles = *cycles + 1;//update cycle counter 
	//next section updates clk register from the io registers
	io_registers[8] += 1;
	if (io_registers[8] >= 0xffffffff)//if full, reset 
		io_registers[8] = 0;
	//next section updates timer registers 
	if (io_registers[11] == 1) {
		if (io_registers[12] < io_registers[13]) { 
			io_registers[12]++;
		}
		else {//reset timer if needed 

			io_registers[12] = 0;
			io_registers[3] = 1;
		}
	}

	//check if diskstatus =1 - cycle_counter++, else cycle_counter=0
	if (io_registers[17] == 1 && *cycle_counter != 1024 && io_registers[14] != 0)
		*cycle_counter = *cycle_counter + 1;
	else if (io_registers[17] != 1)
		*cycle_counter = 0;

	//check if cycle_counter =1024 and if so copy the relevent information 
	if (io_registers[17] == 1 && *cycle_counter == 1024) { //diskstatus =1
		if (io_registers[14] == 1) { //read command
			for (int line_counter = 0;line_counter < 128;line_counter++) {
				memin[io_registers[16] + line_counter] = disk[128 * io_registers[15] + line_counter];
			}
		}
		else if (io_registers[14] == 2) {//write command
			for (int line_counter = 0;line_counter < 128;line_counter++) {
				disk[128 * io_registers[15] + line_counter] = memin[io_registers[16] + line_counter];
			}
		}
		io_registers[17] = 0;//diskstatus =0
		io_registers[14] = 0;//diskcmd =0
		*cycle_counter = 0;
	}

	if (op_code == 0) { //add
		registers[rd] = registers[rs] + registers[rt];
	}
	else if (op_code == 1) { //sub
		registers[rd] = registers[rs] - registers[rt];
	}
	else if (op_code == 2) { // mul 
		registers[rd] = registers[rs] * registers[rt];
	}
	else if (op_code == 3) { //and
		registers[rd] = registers[rs] & registers[rt];
	}
	else if (op_code == 4) { //or
		registers[rd] = registers[rs] | registers[rt];
	}
	else if (op_code == 5) { //xor
		registers[rd] = registers[rs] ^ registers[rt];
	}
	else if (op_code == 6) { //sll
		registers[rd] = registers[rs] << registers[rt];
	}
	else if (op_code == 7) {//aritmetic with sign extension
		int after_shift = registers[rs];
		for (int i = 0; i < registers[rt]; i++) {
			after_shift = after_shift >> 1;

		}
		registers[rd] = after_shift;
	}
	else if (op_code == 8) {//logical shift
		int after_shift = registers[rs];
		for (int i = 0; i < registers[rt]; i++) {
			after_shift = after_shift >> 1;
			after_shift = after_shift & 0x7fffffff; // 7 = 0111, f = 1111
		}
		registers[rd] = after_shift;
	}
	else if (op_code == 9) { //beq
		if (registers[rs] == registers[rt])
			return registers[rd];
	}
	else if (op_code == 10) { //bne
		if (registers[rs] != registers[rt])
			return registers[rd];
	}
	else if (op_code == 11) { //blt
		if (registers[rs] < registers[rt])
			return registers[rd];
	}
	else if (op_code == 12) { // bgt
		if (registers[rs] > registers[rt])
			return registers[rd];
	}
	else if (op_code == 13) { //ble
		if (registers[rs] <= registers[rt])
			return registers[rd];
	}
	else if (op_code == 14) { //bge
		if (registers[rs] >= registers[rt])
			return registers[rd];
	}
	else if (op_code == 15) { //jal
		registers[rd] = pc + 2; // שיניתי פה מלפיסי ועוד 2
		return (registers[rs]);

	}
	else if (op_code == 16) { //lw
		//handle lw
		registers[rd] = memin[registers[rs] + registers[rt]];
		*cycles = *cycles + 1;
		if (io_registers[17] == 1 && *cycle_counter != 1024 && io_registers[14] != 0)
			*cycle_counter = *cycle_counter + 1;
		//next section updates clk register from the io registers
		io_registers[8] += 1;
		if (io_registers[8] >= 0xffffffff)
			io_registers[8] = 0;
		//next section updates timer registers
		if (io_registers[11] == 1) {
			if (io_registers[12] < io_registers[13]) {
				io_registers[12]++;
			}
			else {
				io_registers[12] = 0;
				io_registers[3] = 1;
			}
		}
	}
	else if (op_code == 17) { //sw
		memin[registers[rs] + registers[rt]] = registers[rd];
		int address = (registers[rs] + registers[rt]);
		//printf("address is :%d content:%05X inst:%05X\n", address, registers[rd], memin[pc]);
		*cycles = *cycles + 1;
		if (io_registers[17] == 1 && *cycle_counter != 1024 && io_registers[14] != 0)
			*cycle_counter = *cycle_counter + 1;
		//next section updates clk register from the io registers
		io_registers[8] += 1;
		if (io_registers[8] >= 0xffffffff)
			io_registers[8] = 0;
		//next section updates timer registers
		if (io_registers[11] == 1) {
			if (io_registers[12] < io_registers[13]) {
				io_registers[12]++;
			}
			else {
				io_registers[12] = 0;
				io_registers[3] = 1;
			}
		}
	}
	else if (op_code == 18) { //reti
		//handle reti
		return io_registers[7];//pc = io_registers[7]
	}
	else if (op_code == 19) { // in
		//handle in
		registers[rd] = io_registers[registers[rs] + registers[rt]];
		int reg = registers[rs] + registers[rt];//the io register's number for printing
		print_to_hwregtrace_leds_display7seg_and_update_monitor((*cycles - 1), 0, reg, registers[rd], hwregtrace, leds, display7seg, monitor_array, io_registers);//send to a func to print to hwregtrace
	}
	else if (op_code == 20) {//out
		//handle out
		io_registers[registers[rs] + registers[rt]] = registers[rd];
		int reg = registers[rs] + registers[rt];//the io register's number for printing
		print_to_hwregtrace_leds_display7seg_and_update_monitor((*cycles - 1), 1, reg, registers[rd], hwregtrace, leds, display7seg, monitor_array, io_registers);//send to a func to print to hwregtrace
	}
	else if (op_code == 21) {//halt
		//handle halt - an option for this part if to write pc =-1
		//and check it when going back to the caller
		return -1;

	}
	if ((op_code >= 0 && op_code <= 8) || op_code == 16 || op_code == 19) {
		if (rs == 1 || rt == 1) {
			pc++;
		}
	}
	else if ((op_code >= 9 && op_code <= 14) || op_code == 17 || op_code == 20) {
		if (rd == 1 || rs == 1 || rt == 1) {
			pc++;
		}
	}
	else if (op_code == 15) {
		if (rs == 1) {
			pc++;
		}
	}

	registers[1] = imm;
	registers[0] = 0;
	return pc + 1;
}


//next function prints the currnt content of all registers to trace file 
//updates imm register if needed 
//and then sends them to a function that updates the registers and the pc 
void print_to_trace(int pc, FILE* trace, int* registers, int* io_registers, int* memin, int* cycles, FILE* irq2_file, FILE* hwregtrace, FILE* leds, FILE* display7seg, int* monitor_array, int* disk) {
	int counter = 0;//for test
	int irq = 0;//an int to identify interrupts
	int cycle_counter = 0;
	char* line[MAX_LINE];
	fgets(line, MAX_LINE, irq2_file);//read first line from irq2 file
	int irq2 = atoi(line);//if we have an interrupt from irq2 this int will contain cycle number
	while (pc != -1) {
		int r_or_i = 0;//I type or R type command
		int op_code = 0;
		int rd = 0, rt = 0, rs = 0;
		if (io_registers[5] != 1) {
			if (irq2 == *cycles - 1 || irq2 == *cycles - 2 || irq2 == *cycles - 3) {
				io_registers[5] = 1;//set status to 1 -stil handeling interrupt!
				fgets(line, MAX_LINE, irq2_file);
				irq2 = atoi(line);
			}
			if ((io_registers[0] & io_registers[3]) | (io_registers[1] & io_registers[4]) | (io_registers[2] & io_registers[5])) {
				io_registers[7] = pc;
				pc = io_registers[6];//go to the address in irqhandler
				continue;
			}
		}

		//print PC to trace  
		fprintf(trace, "%03X ", pc);
		//print INST to trace 
		fprintf(trace, "%05X ", memin[pc]);
		if (pc == 128)
			printf("%05X", memin[pc]);
		op_code = ((memin[pc] & 0x000ff000) >> 12);
		rd = ((memin[pc] & 0x00000f00) >> 8);
		rs = ((memin[pc] & 0x000000f0) >> 4);
		rt = ((memin[pc] & 0x0000000f));

		//testing if we have an I type format 
		//and updating io registers (timer and clks)
		if ((op_code >= 0 && op_code <= 8) || op_code == 16 || op_code == 19) {
			if (rs == 1 || rt == 1) {
				registers[1] = memin[pc + 1];
				*cycles = *cycles + 1;
				if (io_registers[17] == 1 && cycle_counter != 1024 && io_registers[14] != 0)
					cycle_counter = cycle_counter + 1;
				io_registers[8] += 1;
				if (io_registers[8] >= 0xffffffff)
					io_registers[8] = 0;
				if (io_registers[11] == 1) {
					if (io_registers[12] < io_registers[13]) {
						io_registers[12]++;
					}
					else {
						io_registers[12] = 0;
						io_registers[3] = 1;
					}
				}
			}
			else
				registers[1] = 0;
		}
		else if ((op_code >= 9 && op_code <= 14) || op_code == 17 || op_code == 20) {
			if (rd == 1 || rs == 1 || rt == 1) {
				registers[1] = memin[pc + 1];
				*cycles = *cycles + 1;
				if (io_registers[17] == 1 && cycle_counter != 1024 && io_registers[14] != 0)
					cycle_counter = cycle_counter + 1;
				io_registers[8] += 1;
				if (io_registers[8] >= 0xffffffff)
					io_registers[8] = 0;
				if (io_registers[11] == 1) {
					if (io_registers[12] < io_registers[13]) {
						io_registers[12]++;
					}
					else {
						io_registers[12] = 0;
						io_registers[3] = 1;
					}
				}
			}
			else
				registers[1] = 0;
		}
		else if (op_code == 15) {
			if (rs == 1) {
				registers[1] = memin[pc + 1];
				*cycles = *cycles + 1;
				if (io_registers[17] == 1 && cycle_counter != 1024 && io_registers[14] != 0)
					cycle_counter = cycle_counter + 1;
				io_registers[8] += 1;
				if (io_registers[8] >= 0xffffffff)
					io_registers[8] = 0;
				if (io_registers[11] == 1) {
					if (io_registers[12] < io_registers[13]) {
						io_registers[12]++;
					}
					else {
						io_registers[12] = 0;
						io_registers[3] = 1;
					}
				}
			}
			else
				registers[1] = 0;
		}
		else
			registers[1] = 0;


		//print all 16 registers to trace- before making the change
		for (int i = 0; i < 15; i++) {
			fprintf(trace, "%08X ", registers[i]);
		}
		fprintf(trace, "%08X", registers[15]);
		fprintf(trace, "\n");

		//next line sends the registers to be changed according to the command
		//and returns the next PC

		pc = change_registers_after_print(registers, pc, io_registers, memin[pc], memin, cycles, hwregtrace, leds, display7seg, monitor_array, disk, &cycle_counter);
		counter++;

	}
}

//next function prints the content of the registers R2-R15 
//after printing to trace.txt the registers contain the updated info
//and therefor we just need to print them as they are 
void print_to_regout(FILE* regout, int* registers) {
	for (int i = 2; i < 16; i++) {
		fprintf(regout, "%08X\n", registers[i]);
	}
	return;
}

// memin_to_memarray gets the array of memin and the memin file
//and copies the files content to the memin (int) array 
void memin_to_array(int memin[4096], FILE* memin_file) {
	char line[10] = "line";
	int stam = 0;
	int test = 0;
	int i = 0;
	while (fgets(line, 7, memin_file) != NULL) {
		stam = sscanf(line, "%X", &memin[i]);
		if (line[0] == '8' || line[0] == '9' || line[0] == 'A' || line[0] == 'a' ||
			line[0] == 'B' || line[0] == 'b' ||
			line[0] == 'C' || line[0] == 'c' ||
			line[0] == 'D' || line[0] == 'd' ||
			line[0] == 'E' || line[0] == 'e' ||
			line[0] == 'F' || line[0] == 'f') {
			test = memin[i];
			test = test | 0xFFF00000;
			memin[i] = test;
		}

		i++;
	}

}

//next lines print the number of cycles to a new file named cycles
void print_to_cycles(FILE* cycles, int cyc) {
	fprintf(cycles, "%d", cyc);
}

//next function prints to memout file 
//the array memin is now updated and finalized and ready to be printed to memout 
void print_to_memout(FILE* memout, int* memin) {
	for (int i = 0; i < 4097; i++) {
		fprintf(memout, "%05X\n", (0x000fffff & memin[i]));
	}
}

//next function prints to monitor.txt and monitor.yuv files
void print_to_monitor(int* monitor_array, FILE* monitor, FILE* monitor_yuv) {
	for (int i = 0; i < 256 * 256; i++) {
		fprintf(monitor, "%02X\n", monitor_array[i]);
		fwrite(&monitor_array[i], 1, 1, monitor_yuv);
	}

	return;
}

//next function gets diskin array and file, and copies diskin file to an int array  
void diskin_to_array(FILE* diskin, int* disk) {
	char line[10] = "line";
	int i = 0;
	int check = 0;
	int test = 0;
	while (fgets(line, 7, diskin) != NULL) {
		check = sscanf(line, "%X", &disk[i]);
		if (line[0] == '8' || line[0] == '9' || line[0] == 'A' || line[0] == 'a' ||
			line[0] == 'B' || line[0] == 'b' ||
			line[0] == 'C' || line[0] == 'c' ||
			line[0] == 'D' || line[0] == 'd' ||
			line[0] == 'E' || line[0] == 'e' ||
			line[0] == 'F' || line[0] == 'f') {
			test = disk[i];
			test = test | 0xFFF00000;
			disk[i] = test;
		}

		i++;
	}


	return;
}

//print to disk - input: diskout file pointer, disk array 
//prints disk array content to diskout file 
//returns void 
void print_to_diskout(FILE* diskout, int* disk) {
	for (int i = 0;i < (128 * 128);i++) {
		fprintf(diskout, "%05X\n", disk[i]);
	}
}

//main - gets argv, argc as inputs 
//main function uses to open and close files and send to other important functions 
//sets all the needed arrays to store memory and registers 
int main(int argc, char* argv[]) {
	FILE* memin = fopen(argv[1], "r");
	int memin_array[4097] = { 0 };
	int disk[128 * 128] = { 0 };
	int pc = 0;//pc counter 
	int registers[16] = { 0 };
	int io_registers[23] = { 0 };
	int cycles = 0;//cycle counter
	int monitor_array[256 * 256] = { 0 };

	FILE* diskin = fopen(argv[2], "r");
	if (diskin == NULL) {//check if file opening worked
		printf("Error Opening diskin.txt");
		exit(1);
	}

	diskin_to_array(diskin, disk);


	FILE* irq2 = fopen(argv[3], "r");
	if (irq2 == NULL) {//check if file opening worked
		printf("Error Opening irq2.txt");
		exit(1);
	}


	FILE* monitor = fopen(argv[12], "w");
	if (monitor == NULL) {
		printf("Error opening monitor file");
		exit(1);
	}
	FILE* monitor_yuv = fopen(argv[13], "w");
	if (monitor_yuv == NULL) {
		printf("Error opening monitor.yuv file");
		exit(1);
	}

	FILE* memin_file = fopen(argv[1], "r");
	if (memin_file == NULL) { printf("error opening file"); exit(1); }
	memin_to_array(memin_array, memin_file);
	fclose(memin_file);

	FILE* trace = fopen(argv[6], "w");
	if (trace == NULL) {
		printf("Error opening trace file");
		exit(1);
	}
	FILE* hwregtrace = fopen(argv[7], "w");
	if (hwregtrace == NULL) {
		printf("Error opening hwregtrace file");
		exit(1);
	}
	FILE* leds = fopen(argv[9], "w");
	if (leds == NULL) {
		printf("Error opening leds file");
		exit(1);
	}
	FILE* display7seg = fopen(argv[10], "w");
	if (leds == NULL) {
		printf("Error opening display7seg file");
		exit(1);
	}
	FILE* diskout = fopen(argv[11], "w");
	if (leds == NULL) {
		printf("Error opening diskout file");
		exit(1);
	}

	print_to_trace(pc, trace, registers, io_registers, memin_array, &cycles, irq2, hwregtrace, leds, display7seg, monitor_array, disk);//write to trace.txt and execute commands
	print_to_monitor(monitor_array, monitor, monitor_yuv);
	print_to_diskout(diskout, disk);
	fclose(trace);
	fclose(irq2);//the file is used in print to trace and thus closed only here 
	fclose(hwregtrace);
	fclose(leds);
	fclose(display7seg);
	fclose(monitor);
	fclose(monitor_yuv);
	fclose(diskout);
	FILE* regout = fopen(argv[5], "w");
	if (regout == NULL) { printf("error opening a file"); exit(1); }
	print_to_regout(regout, registers);//write to regout.txt the content of registers 
	fclose(regout);

	FILE* cycles_file = fopen(argv[8], "w");
	if (cycles_file == NULL) {
		printf("Error opening cycles file");
		exit(1);
	}
	print_to_cycles(cycles_file, cycles);
	fclose(cycles_file);

	FILE* memout = fopen(argv[4], "w");
	if (memout == NULL) {
		printf("Error opening memout file");
		exit(1);
	}
	print_to_memout(memout, memin_array);
	fclose(memout);



	return 0;
}