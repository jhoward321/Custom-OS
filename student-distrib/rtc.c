#include "rtc.h"
#include "i8259.h"
#include "lib.h"


//rtc based off motorola MC146818 - there are some newer variants
//http://lxr.free-electrons.com/source/drivers/char/rtc.c
//https://www.kernel.org/doc/Documentation/rtc.txt


//rate must be above 2 and not over 15
#define RTC_RATE 15
#define RTC_MASK1 0x0F
#define RTC_MASK2 0xF0
#define RTC_IRQ 8

volatile int interrupt_flag; //used to check next interrupt


//code is referenced from link below
//http://wiki.osdev.org/RTC
void rtc_init(void){
	char prev;
	//select register B for reading
	outb(RTC_REG_B, RTC_CMD);
	//store current value stored in register B
	prev = inb(RTC_MEM);
	//reselect B for writing - the read moved it
	outb(RTC_REG_B, RTC_CMD);
	//turn on bit 6 of register B
	outb(prev | RTC_BIT6_EN, RTC_MEM);
	//rate must be above 2 and not over 15
	int rate = RTC_RATE;
	rate &= RTC_MASK1;
 	// select register A for reading
 	cli();
	outb(RTC_REG_A, RTC_CMD);
	prev = inb(RTC_MEM);
	outb(RTC_REG_A, RTC_CMD);
	outb((prev & RTC_MASK2) | rate, RTC_MEM);
	sti();
	// enable_irq(RTC_IRQ);
}
//will need an interrupt handler is in exceptions.c

//return 0 after an interrupt has occurred
int32_t rtc_read(int32_t fd, uint8_t* buf, int32_t length){
	interrupt_flag = 1;

	while(interrupt_flag); //wait for interrupt to occur and then return 0

	return 0;
}

int32_t rtc_write(int32_t fd, uint8_t* buf, int32_t length){
	uint32_t freq = *((uint32_t*) buf);
	int8_t rate;
	char prev;

	if(freq > MAX_FREQ || freq < MIN_FREQ)
		return -1;

	switch(freq){
		case 1024:
			rate = 6;
			break;
		case 512:
			rate = 7;
			break;
		case 256:
			rate = 8;
			break;
		case 128:
			rate = 9;
			break;
		case 64:
			rate = 10;
			break;
		case 32:
			rate = 11;
			break;
		case 16:
			rate = 12;
			break;
		case 8:
			rate = 13;
			break;
		case 4:
			rate = 14;
			break;
		case 2:
			rate = 15;
		case 0:
			rate = 0;
		default:
			return -1; //frequency is not a power of 2 so fails
	}
	rate &= RTC_MASK1;
	cli();
	outb(RTC_REG_A, RTC_CMD);
	prev = inb(RTC_MEM);
	outb(RTC_REG_A, RTC_CMD);
	outb((prev & RTC_MASK2) | rate, RTC_MEM);
	sti();
	return 4;
}


/*The open system call provides access to the file system. The call should find the directory
entry corresponding to thenamed file, allocate an unused file descriptor, and set up any data
necessary to handle the given type of file (directory,RTC device, or regular file). If the
named file does not exist or no descriptors are free, the call returns -1.
*/
//init rtc to 2hz and return 0
int32_t rtc_open(int32_t fd, uint8_t* buf, int32_t length){
	rtc_init();
	return 0;
}

int32_t rtc_close(int32_t fd, uint8_t* buf, int32_t length){
	return 0;
}
