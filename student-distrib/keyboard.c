#include "keyboard.h"
#include "lib.h"
#include "i8259.h"
#include "paging.h"

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


/* Terminal driver notes
when any printable characters typed - display on screen, need to handle all alphanumeric, symbols,
shit/capslocl, dont need to support numpad
now need to track screen location, need to support vertical scrolling
ctrl-L = clear cursor and put it at the top
need to support backspace and line-buffered input - buffer size 128
see appendix B
need external interface to support external data to terminal output
write calls to terminal should interface nicely with keyboard input

read should return data from one line that has been terminated by pressing enter, or as much as fits in
buffer from one such line. Line returned should include the line feed character

write writes data to the terminal. all data should be displayed immediately

writing to terminal needs to briefly block interrupts to update screen data when printing

terminal has 3 states - canonical returns one line of data at a time
noncanonical returns 1 character at a time
third is cbreak

need a open, read, write, close function
*/
//http://www.computer-engineering.org/ps2keyboard/
//#define KB_PORT 0x60 moved to header but left here for reference
uint8_t kbbuf_index;
uint8_t kb_buffer[MAXBUFLEN];
uint8_t out_buffer[MAXBUFLEN];
uint8_t kb_buf_read; //flag for whether or not buffer is ready for reading. 1 is ready, 0 is not
kb_flags_t keyboard_status; //flags for shift, caps lock, etc

//typedef enum {false, true} bool;

//true if want to clear keyboard buffer, false if want to clear out buffer
//true is 1, false is 0
void clear_buffer(int clear_keyboard){
	int i;
	cli();
	if(clear_keyboard){
		for(i = 0; i < MAXBUFLEN; i++)
			kb_buffer[i] = '\0';
		kbbuf_index = 0; //reset index pointer
	}
	else{
		for(i = 0; i < MAXBUFLEN; i++){
			out_buffer[i] = '\0';
		}
		kb_buf_read = 0; //reset flag
	}
	sti();
}

void clear_screen(void){
	clear();
	screen_x = 0;
	screen_y = 0;
	update_cursor(screen_x, screen_y);
	clear_buffer(1);
}
//read data from keyboard, return number of bytes read, read from terminanted line (enter)
//calling terminal read should give me a clear buffer
int32_t terminal_read(int32_t fd, uint8_t* buf, int32_t length){
	if(buf == NULL || length < 0)
		return -1;
	//wait until ready to read
	while(!kb_buf_read){}
	cli();

	memcpy(buf, out_buffer, length < MAXBUFLEN ? length:MAXBUFLEN); //this might need to be < index instead

	sti();
	clear_buffer(0);
	//after reading need to reset buffer index and ready to read
	//kbbuf_index = 0;
	kb_buf_read = 0;

	return length < MAXBUFLEN ? length:MAXBUFLEN;
}
//write data to terminal, display immediately, return number of bytes written or -1 on failure
int32_t terminal_write(int32_t fd, uint8_t* buf, int32_t length){
	
	int byteswritten = 0;

	if((buf == NULL) || (length < 0))
		return -1;
	int i;
	cli();
	for(i = 0; i < length; i++){
		putc(buf[i]); //not sure this is right
	}
	sti();
	return byteswritten;
}

//shouldn't ever get called but needs to exist... returns 0
int32_t terminal_open(int32_t fd, uint8_t* buf, int32_t length){
	return 0;
}
//same as terminal_open
int32_t terminal_close(int32_t fd, uint8_t* buf, int32_t length){
	return 0;
}

//http://wiki.osdev.org/Text_Mode_Cursor
//should only call when a line/string is complete
void update_cursor(int x, int y){
	unsigned short position = (y * 80) + x;
	//cursor LOW port to vga index register
	outb(0x0F, VGA1);
	outb((unsigned char)(position & 0xFF), VGA2);
	//cursor high port to vga index register
	outb(0x0E, VGA1);
	outb((unsigned char)((position >> 8) & 0xFF), VGA2);
}


void keyboard_init(void){
	keyboard_status.ctrl = 0;
	keyboard_status.shift = 0;
	keyboard_status.alt = 0;
	keyboard_status.capslock = 0;
	kb_buf_read = 0;
	kbbuf_index = 0;

	int j;
	for(j = 0; j < MAXBUFLEN; j++){
			kb_buffer[j] = '\0';
	}
	enable_irq(KEYBOARD_IRQ); //enable keyboard interrupts - may need more here but it's a starting point
	//https://www.win.tue.nl/~aeb/linux/kbd/scancodes-11.html#inputport
	//may need to enable keyboard but enabling clock line and clearing bit 4 of the command byte
	//will test interrupt handler first to see if this is necessary

}

//https://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html
//http://www.asciitable.com/


//for checkpoint 1 keyboard handler must echo correct character to screen, doesnt matter where it appears
//referenced http://www.electro.fisica.unlp.edu.ar/temas/lkmpg/node25.html

//handler fills the keyboard buffer and then prints it to terminal
void keyboard_handler(void){
	uint8_t scancode, keycode;
	if(inb(KB_STATUS) & KB_STATUS_MASK){
		scancode = inb(KB_PORT);

		switch(scancode){
			case LCTRL_ON:
				keyboard_status.ctrl = 1;
				break;
			case LCTRL_OFF:
				keyboard_status.ctrl = 0;
				break;
			case LSHIFT_ON:
				keyboard_status.shift = 1;
				break;
			case LSHIFT_OFF:
				keyboard_status.shift = 0;
				break;
			case RSHIFT_ON:
				keyboard_status.shift = 1;
				break;
			case RSHIFT_OFF:
				keyboard_status.shift = 0;
				break;
			case LALT_ON:
				keyboard_status.alt = 1;
				break;
			case LALT_OFF:
				keyboard_status.alt = 0;
				break;
			case CAPSLOCK:
				if(!keyboard_status.capslock)
					keyboard_status.capslock = 1;
				else
					keyboard_status.capslock = 0;
				break;
			case ENTER:
				cli();
				kb_buffer[kbbuf_index] = '\n'; //not sure if we need/want this
				
				putc('\n');
				//copy keyboard buffer to out_buffer for reading
				int i;
				for(i = 0; i < kbbuf_index; i++){
					out_buffer[i] = kb_buffer[i];
				}
				sti();
				//kbbuf_index = 0;
				clear_buffer(1);
				kb_buf_read = 1;
				update_cursor(screen_x, screen_y);
				break;
			case BACKSPACE:
				if(kbbuf_index > 0){
					kbbuf_index--;
					kb_buffer[kbbuf_index] = '\0';

					if(screen_x == 0 && screen_y > 0){
						screen_x = 79;
						screen_y--;
						putc(' ');
						screen_x = 79;
						screen_y--;
					}
					else{
						screen_x--;
						putc(' ');
						screen_x--;//have to decrement cursor again after adding space
					}

					update_cursor(screen_x, screen_y);
				}
				break;

			//still need enter and backspace
			//process scancode and add correct character to buffer
			default:
				if(!(scancode & KB_PRESS_MASK)){

					//ctl L means clear screen
					if(keyboard_status.ctrl && scancode == L){
						//call clear screen
						clear_screen();
						clear_buffer(1);
						//kbbuf_index = 0;
						break;
					}
					//no shift no caps
					else if(!keyboard_status.shift && !keyboard_status.capslock){
						keycode = KBkeys[0][scancode];
					}
					//only shift
					else if(keyboard_status.shift && !keyboard_status.capslock){
						keycode = KBkeys[1][scancode];
					}
					//only capslock
					else if(!keyboard_status.shift && keyboard_status.capslock){
						keycode = KBkeys[2][scancode];
					}
					//capslock and shift
					else if(keyboard_status.shift && keyboard_status.capslock){
						keycode = KBkeys[3][scancode];
					}

					//put into buffer
					if(kbbuf_index < MAXBUFLEN && keycode){
						kb_buffer[kbbuf_index] = keycode;
						kbbuf_index++;
						putc(keycode);
						update_cursor(screen_x, screen_y);
					}
				}
				break;
		}
	}

	send_eoi(KEYBOARD_IRQ); //done with interrupt


}
