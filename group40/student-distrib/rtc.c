#include "rtc.h"
#include "i8259.h"
#include "lib.h"


//rtc based off motorola MC146818 - there are some newer variants
//http://lxr.free-electrons.com/source/drivers/char/rtc.c
//https://www.kernel.org/doc/Documentation/rtc.txt

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

	//now need to set frequency

	//rate buts be above 2 and not over 15
	int rate = 15;
	rate &= 0x0F;
 	// select register A for reading
	outb(RTC_REG_A, RTC_CMD);
	prev = inb(RTC_MEM);
	outb(RTC_REG_A, RTC_CMD);
	outb((prev & 0x0F) | rate, RTC_MEM);

	enable_irq(8); //enable rtc irq

}

//probably will need more rtc functions but this should at least initialize it
//will need an interrupt handle to send to test_interrupts - did this in exceptions.c
