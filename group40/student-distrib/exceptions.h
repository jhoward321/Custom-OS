
#ifndef _EXCEPTIONS_H
#define _EXCEPTIONS_H

#include "types.h"
#include "lib.h"
#include "x86_desc.h"
#include "paging.h"
#include "fs.h"
#include "rtc.h"
#include "keyboard.h"

#define EIGHT_KB 0x2000
#define PCB_ADDR_BASE 0x00800000 		//PCB address for the first task -> bottom of the task 1's kernel stack
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
#define MAGIC_NUM_FOR_EXE0 0x7f
#define MAGIC_NUM_FOR_EXE1 0x45
#define MAGIC_NUM_FOR_EXE2 0x4c
#define MAGIC_NUM_FOR_EXE3 0x46
#define MAGIC_NUM_INDEX0 24
#define MAGIC_NUM_INDEX1 25
#define MAGIC_NUM_INDEX2 26
#define MAGIC_NUM_INDEX3 27
#define INVALID_INODE -1
#define MAX_PCBS 6
#define PCB_END 8
#define PCB_START 2
#define USED 1
#define FREE 0
#define CHAR_BUFF_SIZE 500
#define USER_STACK_ADDR (0x8400000-4)
#define STDIN 0
#define STDOUT 1
#define INVALID -1
#define _128MB 0x08000000
#define _132MB 0x08400000
#define VIRT_VID_INDEX 33 //index in page directory for 132MB
#define MAX_TERMINALS 3


typedef struct operations_table_t {
	int32_t (*read)(int32_t fd, uint8_t* buf, int32_t length);
	int32_t (*write)(int32_t fd, uint8_t* buf, int32_t length);
	int32_t (*open)(int32_t fd, uint8_t* buf, int32_t length);
	int32_t (*close)(int32_t fd, uint8_t* buf, int32_t length);
} operations_table_t;


typedef struct file_descriptor_t{
	operations_table_t* opt;
	int32_t inode_number;
	uint32_t file_position;
	uint32_t flags;
} file_descriptor_t;

typedef struct task_stack_t{
	uint32_t eax, ebx, ecx, edx, esi, edi, esp, ebp, eip, eflags, cr3, esp0, ss0;
} task_stack_t;

typedef struct pcb_t {
	file_descriptor_t file_array[8];
	uint32_t esp; //stores parents esp/ebp which is used in halt
	uint32_t ebp;
	task_stack_t registers;
	struct pcb_t* parent_task;
	struct pcb_t* child_task;
	uint32_t process_id;
	uint32_t eip;
	uint8_t arg[CHAR_BUFF_SIZE];
} pcb_t;

extern pcb_t* curr_task[MAX_TERMINALS];
extern uint32_t pid_used[MAX_TERMINALS][MAX_PCBS];
//extern int current_terminal;

void set_pcbs();

void set_interrupt_gate(uint8_t i);

void set_exeptions();
void ex_error();
void ex_halt();

void ex_0();
void ex_1();
void ex_2();
void ex_3();
void ex_4();
void ex_5();
void ex_6();
void ex_7();
void ex_8();
void ex_9();
void ex_10();
void ex_11();
void ex_12();
void ex_13();
void ex_14();
void ex_15();
void ex_16();
void ex_17();
void ex_18();
void ex_19();

void ex_33(); //keyboard
void ex_40(); //rtc
void ex_128();
void rtc_handler(); //rtc

//10 system call all have 3 arguments but some are not needed so they are garbage
extern int32_t sys_halt(uint8_t status, int32_t garbage2, int32_t garbage3);
extern int32_t sys_execute(const uint8_t* command, int32_t garbage2, int32_t garbage3);
extern int32_t sys_read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t sys_write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t sys_open(const uint8_t* filename, int32_t garbage2, int32_t garbage3);
extern int32_t sys_close(int32_t fd, int32_t garbage2, int32_t garbage3);
extern int32_t sys_getargs(uint8_t* buf, int32_t nbytes, int32_t garbage3);
extern int32_t sys_vidmap(uint8_t** screen_start, int32_t garbage2, int32_t garbage3);
extern int32_t sys_set_handler(int32_t signum, void* handler_address, int32_t garbage3);
extern int32_t sys_sigreturn(int32_t garbage1, int32_t garbage2, int32_t garbage3);

int32_t get_next_pid();
int32_t new_pcb(int8_t* arguments);

#endif
