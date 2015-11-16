#ifndef RTC_H
#define RTC_H
#include "types.h"


//ports used
#define RTC_REG_B 0x8B //address on chip is 0x0B but its on irq8
#define RTC_CMD 0x70
#define RTC_MEM 0x71
#define RTC_BIT6_EN 0x40
#define RTC_REG_A 0x8A
#define RTC_REG_C 0x0C
#define MAX_FREQ 1024
#define MIN_FREQ 2

volatile int interrupt_flag; //used to tell when interrupts occur


//rtc initialization function
void rtc_init(void);
int32_t rtc_read(int32_t fd, uint8_t* buf, int32_t length);
int32_t rtc_write(int32_t fd, uint8_t* buf, int32_t length);
int32_t rtc_close(int32_t fd, uint8_t* buf, int32_t length);
int32_t rtc_open(int32_t fd, uint8_t* buf, int32_t length);


#endif /* RTC_H */
