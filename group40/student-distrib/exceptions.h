
#ifndef _EXCEPTIONS_H
#define _EXCEPTIONS_H

#include "types.h"
#include "lib.h"
#include "x86_desc.h"

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

extern void ex_33(); //keyboard
extern void ex_40(); //rtc
void rtc_handler(); //rtc

void ex_128(); //rtc

#endif
