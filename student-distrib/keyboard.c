#include "keyboard.h"
#include "lib.h"
#include "i8259.h"

//https://www.kernel.org/pub/linux/kernel/people/marcelo/linux-2.4/drivers/char/keyboard.c
//http://www.electro.fisica.unlp.edu.ar/temas/lkmpg/node25.html
//https://www.win.tue.nl/~aeb/linux/kbd/scancodes-11.html

//keyboard is on IRQ1 on intel architectures

//when receive interrupt - read keyboard status and scan code

//code of key is first 7 bits of scan code, 8th bit is key status (ie pressed or not)
//and scancode with x80 to check high order bit - if 1 keys been pressed, 0 its up
//low order bit = 1 then key has been toggled (ie capslock)
//https://stackoverflow.com/questions/2746817/what-does-the-0x80-code-mean-when-referring-to-keyboard-controls
/*
	keyboard has 3 8-bit registers involved with cpu. input buffer (write to 0x60 or 0x64), output buffer
	 can be read by reading from port 0x60, and status register which is read at port x64
	 if cpu writes to x64 - interpreted as command byte
	 if cpu writes to port x60 - byte interpreted as data byte

	 keyboard has 2 8-bit i/o ports involved in communicating with keyboard: input port P1 (receive input from keyboard)
	 and output port P2 (for sending output to the keyboard)

	see https://www.win.tue.nl/~aeb/linux/kbd/scancodes-11.html#inputport for controller commands
*/

//#define KB_PORT 0x60 moved to header but left here for reference
uint8_t kb_index;
uint8_t kb_in_buffer[MAXBUFLEN];

void keyboard_init(void){
	enable_irq(1); //enable keyboard interrupts - may need more here but it's a starting point
	//https://www.win.tue.nl/~aeb/linux/kbd/scancodes-11.html#inputport
	//may need to enable keyboard but enabling clock line and clearing bit 4 of the command byte
	//will test interrupt handler first to see if this is necessary
}

//https://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html
//http://www.asciitable.com/
unsigned char KBkeys[MAXBUFLEN] =
{
	//00 and 01 are error and ESC
	0, //error code
	27, //ESC
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', //0-9, -, =
	'\b',//backspace
	'\t',//tab - index 15
	'q','w','e','r','t','y','u','i','o','p','[',']', //index 27
	'\n', //enter key (index 1c = 28)
	0, //left control but dont care about for checkpoint 1
	'a','s','d','f','g','h','j','k','l',';','\'', //index 40
	'`',
	0, //left shift we dont care about for checkpoint 1
	'\'','z','x','c','v','b','n','m',',','.','/', //index 54
	0, //right shift
	'*',
	0, //left alt
	' ', //space
	0,//capslock
	0,0,0,0,0,0,0,0,0,0,//F1-F10 index 69
	0, 0, //numlock, scrolllock
	0,0,0, //home, up, pgup
	'-', //numpad minus
	0,0,0,'+', //keypad left, 5, right, +, index 79
	0,0,0, //numpad end, down, pgdown
	0,0, //insert, delete
	0,0,0, //dont usually occur but reference site for explanation - can leave as 0 most likely
	0,0, //f11, f12 index 89
	0, //rest are all 0
};

//for checkpoint 1 keyboard handler must echo correct character to screen, doesnt matter where it appears
//referenced http://www.electro.fisica.unlp.edu.ar/temas/lkmpg/node25.html
void keyboard_handler(void){
	uint8_t scancode, status, keycode;

	//read keyboard status
	//status = inb(KB_STATUS);
	scancode = inb(KB_PORT);

	//have to check high order bit to check if key is up or down (and with x80 to get high order bit)
	//if 0 the key is down, if 1 its been released
	if(!(scancode & KB_PRESS_MASK)){
		keycode = KBkeys[scancode];
		kb_in_buffer[kb_index] = keycode;
		kb_index++;
	}
	send_eoi(1); //done with interrupt


}
