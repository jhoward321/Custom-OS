#ifndef RTC_H
#define RTC_H

//ports used
#define RTC_REG_B 0x8B //address on chip is 0x0B but its on irq8
#define RTC_CMD 0x70
#define RTC_MEM 0x71
#define RTC_BIT6_EN 0x40
#define RTC_REG_A 0x8A
#define RTC_REG_C 0x0C

//rtc initialization function
void rtc_init(void);


#endif /* RTC_H */
