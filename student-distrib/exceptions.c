#include "exceptions.h"
#include "rtc.h" //needed for rtc handler
#include "i8259.h"
#include "keyboard.h"
#include "x86_desc.h"
#include "lib.h"
#include "fs.h"

#define KEYBOARD_IDT 33 			//Keyboard IDT value
#define RTC_IDT 40 					//RTC IDT value
#define SYSTEM_CALL_IDT 128 		//System Call IDT value
#define DPL_SYS 3 					//DPL value necessary for system call situations
#define READ 0
#define WRITE 1
#define OPEN 2
#define CLOSE 3
#define MAX_OPEN_FILES 8
#define VIRT_ADDR128_INDEX 0x20		//32 is index in page directory for 128MB virtual address
#define PROG_EXEC_ADDR 0x08048000
#define EIGHT_MB 0x0800000
#define IF_FLAG 0x200


pcb_t* curr_task;

static uint32_t pid_used[6] = {0,0,0,0,0,0};
static uint32_t PCB_ADDR[6] = {
						PCB_ADDR0,
						PCB_ADDR1,
						PCB_ADDR2,
						PCB_ADDR3,
						PCB_ADDR4,
						PCB_ADDR5
};

// int32_t (*file_operations[])(int32_t fd, uint8_t* buf, int32_t length) = {read_file, write_file, open_file, close_file};
// int32_t (*dir_operations[])(int32_t fd, uint8_t* buf, int32_t length)  = {read_dir, write_dir, open_dir, close_dir};
// int32_t (*rtc_operations[])(uint32_t freq)  = {rtc_read, rtc_write, rtc_open, rtc_close};
// int32_t (*stdin_operations[])(uint8_t* buf, int32_t nbytes) = {terminal_read, NULL, terminal_open, terminal_close};
// int32_t (*stdout_operations[])(uint8_t* buf, int32_t nbytes) = {NULL, terminal_write, NULL, NULL};

operations_table_t file_operations = {read_file, write_file, open_file, close_file};
operations_table_t dir_operations = {read_dir, write_dir, open_dir, close_dir};
operations_table_t rtc_operations = {rtc_read, rtc_write, rtc_open, rtc_close};
operations_table_t stdin_operations = {terminal_read, NULL, terminal_open, terminal_close};
operations_table_t stdout_operations = {NULL, terminal_write, NULL, NULL};



void set_exeptions(){
	//20 interrupts defined by intel
	//32 reserved by intel but only 20 are defined


	SET_IDT_ENTRY(idt[0], ex_0);
	SET_IDT_ENTRY(idt[1], ex_1);
	SET_IDT_ENTRY(idt[2], ex_2);
	SET_IDT_ENTRY(idt[3], ex_3);
	SET_IDT_ENTRY(idt[4], ex_4);
	SET_IDT_ENTRY(idt[5], ex_5);
	SET_IDT_ENTRY(idt[6], ex_6);
	SET_IDT_ENTRY(idt[7], ex_7);
	SET_IDT_ENTRY(idt[8], ex_8);
	SET_IDT_ENTRY(idt[9], ex_9);
	SET_IDT_ENTRY(idt[10], ex_10);
	SET_IDT_ENTRY(idt[11], ex_11);
	SET_IDT_ENTRY(idt[12], ex_12);
	SET_IDT_ENTRY(idt[13], ex_13);
	SET_IDT_ENTRY(idt[14], ex_14);
	SET_IDT_ENTRY(idt[15], ex_15);
	SET_IDT_ENTRY(idt[16], ex_16);
	SET_IDT_ENTRY(idt[17], ex_17);
	SET_IDT_ENTRY(idt[18], ex_18);
	SET_IDT_ENTRY(idt[19], ex_19);

	SET_IDT_ENTRY(idt[KEYBOARD_IDT], ex_33);
	//SET_IDT_ENTRY(idt[33], keyboard_handler);	//keyboard - moved handler to keyboard.c
	SET_IDT_ENTRY(idt[RTC_IDT], ex_40);	//RTC


	SET_IDT_ENTRY(idt[SYSTEM_CALL_IDT], ex_128);//system call

	//for loop below sets up first 20 interrupt handlers
	uint8_t i;
	for(i = 0; i < 20; i++){
		set_interrupt_gate(i);
	}
	set_interrupt_gate(KEYBOARD_IDT);
	set_interrupt_gate(RTC_IDT);
	set_interrupt_gate(SYSTEM_CALL_IDT); //this needs a different dpl value since needs to be accessed by user space
}

void set_interrupt_gate(uint8_t i){
	idt[i].seg_selector 	= KERNEL_CS;
	idt[i].reserved4 	= 0x00;
	idt[i].reserved3 	= 0;
	idt[i].reserved2 	= 1;
	idt[i].reserved1 	= 1;
	idt[i].size 		= 1;	//side is D, 1 = 32 bits
	idt[i].reserved0	= 0;
	if(i == SYSTEM_CALL_IDT)
		idt[i].dpl 	= DPL_SYS; 		//necessary DPL value for system call
	else
		idt[i].dpl 	= 0; 		//default DPL value when no system call
	idt[i].present 		= 1;
}


void ex_error(){
	disable_irq(1);			//disable keyboard Interrupts
	clear();			//clear the screen
	printf("Error #");		//let user know there is an error

}
void ex_halt(){				//loop on halt
	while(1){}
}



void ex_0(){
	ex_error();
	printf("0: Divide by zero\n");
	ex_halt();
}
void ex_1(){
	ex_error();
	printf("1: Debug\n");
	ex_halt();
}
void ex_2(){
	ex_error();
	printf("2: Nonmaskable Interrupts (NMI)\n");
	ex_halt();
}
void ex_3(){
	ex_error();
	printf("3: Breakpoint\n");
	ex_halt();
}
void ex_4(){
	ex_error();
	printf("4: Overflow\n");
	ex_halt();
}
void ex_5(){
	ex_error();
	printf("5: Bounds check\n");
	ex_halt();
}
void ex_6(){
	ex_error();
	printf("6: Invalid opcode\n");
	ex_halt();
}
void ex_7(){
	ex_error();
	printf("7: Device not available\n");
	ex_halt();
}
void ex_8(){
	ex_error();
	printf("8: Double fault\n");
	ex_halt();
}
void ex_9(){
	ex_error();
	printf("9: Coprocessor segment overrun\n");
	ex_halt();
}
void ex_10(){
	ex_error();
	printf("10: Invalid TSS\n");
	ex_halt();
}
void ex_11(){
	ex_error();
	printf("11: Segment not present\n");
	ex_halt();
}
void ex_12(){
	ex_error();
	printf("12: Stack segment\n");
	ex_halt();
}
void ex_13(){
	ex_error();
	printf("13: General protection\n");
	ex_halt();
}
void ex_14(){
	ex_error();
	printf("14: Page Fault\n");
	ex_halt();
}
void ex_15(){
	ex_error();
	printf("15: reserved?\n");
	ex_halt();
}
void ex_16(){
	ex_error();
	printf("16: Floating-point error\n");
	ex_halt();
}
void ex_17(){
	ex_error();
	printf("17: Alignment check\n");
	ex_halt();
}
void ex_18(){
	ex_error();
	printf("18: Machine check\n");
	ex_halt();
}
void ex_19(){
	ex_error();
	printf("19: SIMD floating point\n");
	ex_halt();
}


//referenced code from this site
//http://wiki.osdev.org/RTC#Interrupts_and_Register_C
void rtc_handler(){	//RTC
	//have to read register C to allow interrupt to happen again

	outb(RTC_REG_C, RTC_CMD);
	//dont care about contents
	inb(RTC_MEM);

	test_interrupts(); //for checkpoint 1 - in lib.c

	send_eoi(RTC_IRQ); //interrupt is over
	interrupt_flag = 0; //clear flag now that interrupt is over
}



// operations_table_t file_operations = {read_file, write_file, open_file, close_file};
// operations_table_t dir_operations = {read_dir, write_dir, open_dir, close_dir};
// operations_table_t rtc_operations = {rtc_read, rtc_write, rtc_open, rtc_close};
// operations_table_t stdin_opt = {};
// operations_table_t stdout_opt;


//checkpoint 3 need halt, execute, open, close, read, write
//successful calls return 0, failed calls return -1
//call number placed in eax, first arg in ebx, ecx, edx. No call uses more than
//3 arguments. Return value placed in eax

//input is 8 bit value indicating status of current process
//should never return to the caller
int32_t sys_halt(uint8_t status, int32_t garbage2, int32_t garbage3){
	//if process being killed is pid0, start shell again
	//halt terminates a process, returning the specified value to its parent process
	if(curr_task->parent_task == NULL){
		sys_execute((uint8_t*)"shell", 0,0);
	}

	pid_used[curr_task->process_id] = 0; //no longer used
	//store parents ebp/esp values before changing curr_task
	uint32_t parentebp = curr_task->ebp;
	uint32_t parentesp = curr_task->esp;
	uint32_t parenteip = curr_task->eip;
	curr_task = curr_task->parent_task;
	curr_task->child_task = NULL;

	//restore parents paging
	uint32_t pde = calc_pde_val(curr_task->process_id);
	add_page(pde, VIRT_ADDR128_INDEX);
	//set cr3 register - flush TLB
	reset_cr3();

	tss.esp0 = EIGHT_MB - (curr_task->process_id * EIGHT_KB) - 4;
	//jmp halt_ret_label

	//restore old ebp/esp values
	asm volatile(
		"movl %0, %%esp \n\
		movl %1, %%ebp \n\
		pushl %2 \n\
		ret \n\
		"
		:
		:"r"(parentesp), "r"(parentebp), "r"(parenteip)
	);

	return -1; //should never get here
}


//attempts to load and execute new program until it terminates
//command is space separated sequence of words - first word is file name of program
//rest of command - stripped of leading spaces, is provided to program on request via getargs syscall
//returns -1 if cant be returned - ie program doesnt exist, not executable,
//return 256 if program dies by an exception
//return value in range 0 to 255 if program executes halt syscall, val returned is given by programs call
//to halt
int32_t sys_execute(const uint8_t* command, int32_t garbage2, int32_t garbage3){
	/*parse, exe check, set up paging, file loader, new pcb,
	context switch - write tss.esp0/ebp0 with new process kernel stack?
		save current esp/ebp or anything needed in pcb
		push artificial IRET context onto stack
		IRET
		halt_ret_label?
	RET
	*/

	if(command == NULL)
		return -1;
	//parse name of program and arguments
	uint32_t i;
	uint8_t argsflag = 0;
	for(i = 0; i < strlen((int8_t *)command); i++){
		if(command[i] == ' '){
			//check for args and get index where command ends and args start
			argsflag = 1;
			break;
		}
	}
	//if flag has been set then we have arguments and need to parse
	char program[500];
	char arguments[500];
	if(argsflag == 1){
		//arguments exist, need to parse out arguments
		strncpy(program, (int8_t *) command, i);
		program[i] = '\0';
		i++; //increment index to start of args rather than where first space is
		strncpy((int8_t *)arguments, (int8_t *) (command + i), strlen((int8_t *)command) - i);
	}
	//no args
	else{
		strncpy(program, (int8_t *)command, strlen((int8_t *) command) + 1); //+1 copies over null terminator
		arguments[0] = '\0';
	}
	//done parsing arguments, make sure executable
	//make sure file exists
	dentry_t fileinfo;
	if(read_dentry_by_name((uint8_t *) program, &fileinfo) == -1)
		return -1;
	//file exists, make sure executable
	unsigned char buffer[4];
	read_data(fileinfo.inode_number, 0, buffer, 4);
	if(buffer[0] != 0x7f || buffer[1] != 0x45 || buffer[2] != 0x4c || buffer[3] != 0x46)
		return -1;
	//if reach here file exists and is executable
	//set up paging
	uint32_t pde;
	if(curr_task==NULL)
		pde = calc_pde_val(0);
	else
		pde = calc_pde_val(curr_task->process_id);
	add_page(pde, VIRT_ADDR128_INDEX);

	//set cr3 register
	reset_cr3();

	//File Loader
	uint8_t *progbuf = (uint8_t*) PROG_EXEC_ADDR;
	uint32_t filelength = read_file_length(fileinfo.inode_number);
	read_data(fileinfo.inode_number, 0, progbuf, filelength);

	//New PCB
	new_pcb();
	//context switch

	//need to get execution point - stored li
	curr_task->eip = (progbuf[27] << 24) + (progbuf[26] << 16) + (progbuf[25] << 8) + (progbuf[24]);
	//curr_task->eip = read_data(fileinfo.inode_number, 0, (uint8_t*)&curr_task->eip, 4);

	//need to save old ebp/esp into pcb
	asm volatile(
		"movl %%esp, %0"
		:"=r"(curr_task->esp)
	);
	asm volatile(
		"movl %%ebp, %0"
		:"=r"(curr_task->ebp)
	);

	//set tss stuff
	tss.ss0 = KERNEL_DS;
	tss.esp0 = EIGHT_MB - (curr_task->process_id * EIGHT_KB); //see kernel.c, x86_desc for tss info

	uint32_t user_stack = 0x8400000-4;
	//push IRET context onto stack, not positive my eip/esp values are correct
	asm volatile(
		"movl %0, %%eax \n\
		movw %%ax, %%ds \n\
		pushl %%eax \n\
		pushl %1 \n\
		pushfl \n\
		orl %2, (%%esp) \n\
		pushl %3 \n\
		pushl %4 \n\
		iret \n\
		"
		:
		: "r" (USER_DS), "r" (user_stack), "r" (IF_FLAG), "r" (USER_CS), "r" (curr_task->eip)
		: "eax", "memory", "cc"
	);


	//IRET, halt_ret_label, RET
	asm volatile("HALT_RET_LABEL:");

	return 0;
}

int32_t sys_read(int32_t fd, void* buf, int32_t nbytes){

	// fd has to be in range AND fd cannot be 1 (stdout)
	if(fd < 0 || fd > 7 || fd == 1 || nbytes < NULL || nbytes <= 0 || curr_task->file_array[fd].flags == 0)
		return -1;


	return curr_task->file_array[fd].opt->read(fd, buf, nbytes);
}

int32_t sys_write(int32_t fd, const void* buf, int32_t nbytes){

	if(fd <= 0 || fd > 7 || curr_task->file_array[fd].flags == 0)
		return -1;

	return curr_task->file_array[fd].opt->write(fd, (uint8_t*)buf, nbytes);

}

int32_t sys_open(const uint8_t* filename, int32_t garbage2, int32_t garbage3){

	dentry_t temp;
	uint32_t fd;
	uint32_t curr_available = -1;
	//check if file exists
	if (read_dentry_by_name(filename, &temp) == -1){
		return -2; 				//return value for file doesn't exist
	}

	for(fd = 2; fd<8; fd++){ 		//go through the file array for the current pcb
		if(curr_task->file_array[fd].flags == 0){
			curr_available = fd;
			break;
		}
	}

	if(curr_available == -1)
		return -1;

	switch(temp.file_type){
		case 0:
			curr_task->file_array[curr_available].opt =  &rtc_operations;
			rtc_open(0, NULL, 0);
			break;

		case 1:
			curr_task->file_array[curr_available].opt = &dir_operations;  //CHECK THIS <====================
			open_dir(curr_available, NULL, 0);
			break;

		case 2:
			curr_task->file_array[curr_available].opt = &file_operations;  //CHECK THIS <====================
			open_file(curr_available, NULL, 0);
			break;

		default:
			curr_task->file_array[curr_available].opt = &stdin_operations;
			terminal_open(0, NULL, 0);
			break;

	}

	return curr_available;
}

int32_t sys_close(int32_t fd, int32_t garbage2, int32_t garbage3){

	return -1;
}

int32_t sys_getargs(uint8_t* buf, int32_t nbytes, int32_t garbage3){

	return -1;
}

int32_t sys_vidmap(uint8_t** screen_start, int32_t garbage2, int32_t garbage3){

	return -1;
}

int32_t sys_set_handler(int32_t signum, void* handler_address, int32_t garbage3){

	return -1;
}

int32_t sys_sigreturn(int32_t garbage1, int32_t garbage2, int32_t garbage3){

	return -1;
}

int32_t get_next_pid(){
	int i;
	for(i=0; i<6; i++){
		if(pid_used[i] == 0)
			return i;
	}
	return -1;
}


int32_t new_pcb(){
	int next_pid = get_next_pid();
	int i;

	if(next_pid == -1)
		return -1;

	pid_used[next_pid] = 1; 		//set pid to being used

	pcb_t* retval = (pcb_t*) PCB_ADDR[next_pid];

	for(i=2; i<8; i++){
		retval->file_array[i].opt = NULL;
		retval->file_array[i].inode_number = -1; 		//set an invalid value
		retval->file_array[i].file_position = 0; 	//position 0
		retval->file_array[i].flags = 0; 	//not in use
	}

	//set stdin:
	retval->file_array[0].opt = &stdin_operations;
	retval->file_array[0].inode_number = -1;
	retval->file_array[0].file_position = 0;
	retval->file_array[0].flags = 1;

	//set stdout:
	retval->file_array[1].opt = &stdout_operations;
	retval->file_array[1].inode_number = -1;
	retval->file_array[1].file_position = 0;
	retval->file_array[1].flags = 1;

	if(curr_task == NULL){ 			// !!!!!!============SET CURR_TASK TO NULL IN KERNEL.C
		retval->parent_task = NULL;
		retval->child_task = NULL;
		retval->process_id = next_pid;
	}
	else{
		curr_task->child_task = retval;
		retval->parent_task = curr_task;
		retval->child_task = NULL;
		retval->process_id = next_pid;
	}

	curr_task = retval;
	//esp and ebp not set.

	return next_pid;

}
