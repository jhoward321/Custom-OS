#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "types.h"

#define KB_PORT 0x60 			//I/O port number for keyboard
#define KB_STATUS 0x64 			//not-used for this checkpoint
#define KB_PRESS_MASK 0x80 		//mask to check if a key is pressed
#define MAXBUFLEN 128 			//character buffer length, used for I/O

extern uint8_t kb_index;
extern uint8_t kb_in_buffer[MAXBUFLEN]; //this will probably need to be modified for checkpoint 2 which specifies the 128 length buffer
//uint8_t kb_out_buffer[MAXBUFLEN];



void keyboard_init(void); //not sure if this is even needed
void keyboard_handler(void); //exception handler for keyboard


#endif /* KEYBOARD_H */
