#include "exceptions.h"
#include "rtc.h" //needed for rtc handler
#include "i8259.h"
#include "keyboard.h"
#include "lib.h"

void set_exeptions(){
	//20 interrupts defined by intel
	//32 reserved by intel but only 20 are defined

	disable_irq(1);		//disable keyboard during first 19 exceptions
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
	enable_irq(1);			//re-enable keyboard

	//SET_IDT_ENTRY(idt[33], ex_33);
	SET_IDT_ENTRY(idt[33], keyboard_handler);	//keyboard - moved handler to keyboard.c
	SET_IDT_ENTRY(idt[40], ex_40);	//RTC


	SET_IDT_ENTRY(idt[128], ex_128);//system call

	//for loop below sets up first 20 interrupt handlers
	uint8_t i;
	for(i = 0; i < 20; i++){
		set_interrupt_gate(i);
	}
	set_interrupt_gate(33);
	set_interrupt_gate(40);
	set_interrupt_gate(128); //this needs a different dpl value since needs to be accessed by user space
}

void set_interrupt_gate(uint8_t i){
	idt[i].seg_selector 	= KERNEL_CS;
	idt[i].reserved4 	= 0x00;
	idt[i].reserved3 	= 0;
	idt[i].reserved2 	= 1;
	idt[i].reserved1 	= 1;
	idt[i].size 		= 1;	//side is D, 1 = 32 bits
	idt[i].reserved0	= 0;
	if(i == 128)
		idt[i].dpl 	= 3;
	else
		idt[i].dpl 	= 0;
	idt[i].present 		= 1;
}


void ex_error(){
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

/*
void ex_33(){	//keyboard - handler moved to keyboard.c
	clear();
	printf("keyboard handler called\n");
}
*/

//referenced code from this site
//http://wiki.osdev.org/RTC#Interrupts_and_Register_C
void ex_40(){	//RTC
	//have to read register C to allow interrupt to happen again
	outb(RTC_REG_C, RTC_CMD);
	//dont care about contents
	inb(RTC_MEM);

	test_interrupts(); //for checkpoint 1 - in lib.c

	send_eoi(8); //interrupt is over

}

void ex_128(){	//system call

}
