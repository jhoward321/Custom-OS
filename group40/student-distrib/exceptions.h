
#ifndef _EXCEPTIONS_H
#define _EXCEPTIONS_H

#include "types.h"
#include "lib.h"
#include "x86_desc.h"

typedef struct operations_table_t {
	int32_t (*read)(int32_t fd, void* buf, int32_t length);
	int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
	int32_t (*open)(int32_t fd);
	int32_t (*close) (int32_t fd);
} operations_table_t;

typedef struct file_descriptor_t{
	operations_table_t* opt;
	uint32_t inode_pointer;
	uint32_t file_position;
	uint32_t flags;
} file_descriptor_t;

typedef struct pcb_t {
	file_descriptor_t file_array[8];
	uint32_t esp;
	uint32_t ebp;
} pcb_t;




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

#endif
