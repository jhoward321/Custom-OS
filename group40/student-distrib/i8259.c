/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

//referenced http://wiki.osdev.org/8259_PIC

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts
 * are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7 */
uint8_t slave_mask; /* IRQs 8-15 */

#define INIT_MASK 0xFF 				//bitmask to mask all IRQ lines
#define PIC_PORT_LIM 8 				//number of IRQs for each PIC

/*
* void i8259_init(void)
*   Inputs: none
*   Return Value: none
*	Function: Initialize the 8259 PIC
*/
void
i8259_init(void)
{
	//mask all the lines
	outb(INIT_MASK, MASTER_8259_DATA);
	outb(INIT_MASK, SLAVE_8259_DATA);

	//start initialization - master first
	outb(ICW1, MASTER_8259_PORT);
	outb(ICW2_MASTER, MASTER_8259_DATA); //maps irq 0-7
	outb(ICW3_MASTER, MASTER_8259_DATA); //slave is on IRQ2
	outb(ICW4, MASTER_8259_DATA);

	//init slave
	outb(ICW1, SLAVE_8259_PORT);
	outb(ICW2_SLAVE, SLAVE_8259_DATA); //map irq's
	outb(ICW3_SLAVE, SLAVE_8259_DATA); //tell slave that its a slave of master on irq2
	outb(ICW4, SLAVE_8259_DATA);

	outb(INIT_MASK, MASTER_8259_DATA);
	outb(INIT_MASK, SLAVE_8259_DATA);

}

/*
* void enable_irq(uint32_t irq_num)
*   Inputs: none
*   Return Value: none
*	Function: Enable (unmask) the specified IRQ
*/
void
enable_irq(uint32_t irq_num)
{
	uint16_t port;
	uint8_t value;

	//irq is on master PIC
	if(irq_num < PIC_PORT_LIM)
		port = MASTER_8259_DATA;

	//irq is on slave PIC
	else{
		port = SLAVE_8259_DATA;
		irq_num -= PIC_PORT_LIM;
	}
	value = inb(port) & ~(1 << irq_num);
	outb(value, port);

}

/*
* void disable_irq(uint32_t irq_num)
*   Inputs: none
*   Return Value: none
*	Function: Disable (mask) the specified IRQ
*/
void
disable_irq(uint32_t irq_num)
{
	uint16_t port;
	uint8_t value;

	if(irq_num < PIC_PORT_LIM)
		port = MASTER_8259_DATA;
	else{
		port = SLAVE_8259_DATA;
		irq_num -= PIC_PORT_LIM;
	}
	value = inb(port) | (1 << irq_num);
	outb(value, port);
}


/*
* void send_eoi(uint32_t irq_num)
*   Inputs: none
*   Return Value: none
*	Function: Send end-of-interrupt signal for the specified IRQ
*/
void
send_eoi(uint32_t irq_num)
{
	
	if(irq_num >= PIC_PORT_LIM){
		//have to get the actual EOI and pass it to slave if originated on slave PIC
		outb(EOI | (irq_num - PIC_PORT_LIM) , SLAVE_8259_PORT);
		//notify master on irq2(where the slave is)
		outb(EOI | 2, MASTER_8259_PORT);
	}
	else{
		//notify master PIC
		outb(EOI | irq_num, MASTER_8259_PORT);
	}

}
