#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "types.h"

#define KB_PORT 0x60 			//I/O port number for keyboard
#define KB_STATUS 0x64 			//not-used for this checkpoint
#define KB_STATUS_MASK 0x01
#define KB_PRESS_MASK 0x80 		//mask to check if a key is pressed
#define MAXBUFLEN 128 			//character buffer length, used for I/O
//#define TERMINAL_BUF_LEN 1024
#define KBKEY_ARRAY 4
//#define NUM_TERMINALS 3
#define FOURKB 4096

//special keycodes
#define BACKSPACE 0x0E
#define TAB 0x0F
#define ENTER 0x1C
#define LCTRL_ON 0x1D
#define LCTRL_OFF 0x9D
#define LSHIFT_ON 0x2A
#define LSHIFT_OFF 0xAA
#define RSHIFT_ON 0x36
#define RSHIFT_OFF 0xB6
#define LALT_ON 0x38
#define LALT_OFF 0xB8
#define CAPSLOCK 0x3A
#define L 0x26
#define C 0x2E

//copied from lib.c
#define VIDEO 0xB8000
#define NUM_COLS 80
#define NUM_ROWS 25
#define ATTRIB 0x7
#define VGA1 0x3D4
#define VGA2 0x3D5

extern uint8_t kb_index;
extern uint8_t kb_in_buffer[MAXBUFLEN]; //this will probably need to be modified for checkpoint 2 which specifies the 128 length buffer
uint8_t kb_out_buffer[MAXBUFLEN];

typedef struct kb_flags_t{
	//flags for keyboard state
	uint8_t ctrl : 1;
	uint8_t shift : 1;
	uint8_t capslock : 1;
	//uint8_t backspace : 1;
	uint8_t alt : 1;
} kb_flags_t;

// typedef struct terminal_t{
// 	uint32_t cursor_x;
// 	uint32_t cursor_y;
// }terminal_t;

//4 for noshift/caps, shift, caps, caps+shift
static const uint8_t KBkeys[KBKEY_ARRAY][MAXBUFLEN] =
{
	//no caps/shift
	{
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
	'\\','z','x','c','v','b','n','m',',','.','/', //index 54
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
	0 //rest are all 0
	},

	//Shift
	{0, //error code
	27, //ESC
	'!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', //0-9, -, =
	'\b',//backspace
	'\t',//tab - index 15
	'Q','W','E','R','T','Y','U','I','O','P','{','}', //index 27
	'\n', //enter key (index 1c = 28)
	0, //left control but dont care about for checkpoint 1
	'A','S','D','F','G','H','J','K','L',':','\"', //index 40
	'~',
	0, //left shift we dont care about for checkpoint 1
	'|','Z','X','C','V','B','N','M','<','>','?', //index 54
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
	0 //rest are all 0
	},

	//Capslock
	{0, //error code
	27, //ESC
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', //0-9, -, =
	'\b',//backspace
	'\t',//tab - index 15
	'Q','W','E','R','T','Y','U','I','O','P','[',']', //index 27
	'\n', //enter key (index 1c = 28)
	0, //left control but dont care about for checkpoint 1
	'A','S','D','F','G','H','J','K','L',';','\'', //index 40
	'`',
	0, //left shift we dont care about for checkpoint 1
	'\\','Z','X','C','V','B','N','M',',','.','/', //index 54
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
	0 //rest are all 0
	},

	//Capslock & shift
	{0, //error code
	27, //ESC
	'!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', //0-9, -, =
	'\b',//backspace
	'\t',//tab - index 15
	'q','w','e','r','t','y','u','i','o','p','{','}', //index 27
	'\n', //enter key (index 1c = 28)
	0, //left control but dont care about for checkpoint 1
	'a','s','d','f','g','h','j','k','l',':','\"', //index 40
	'~',
	0, //left shift we dont care about for checkpoint 1
	'\\','z','x','c','v','b','n','m','<','>','?', //index 54
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
	0 //rest are all 0
	}
};

void keyboard_init(void); //not sure if this is even needed
void keyboard_handler(void); //exception handler for keyboard
void clear_screen(void);
void clear_buffer(int clear_keyboard);
int32_t terminal_read(int32_t fd, uint8_t* buf, int32_t length);
int32_t terminal_write(int32_t fd, uint8_t* buf, int32_t length);
int32_t terminal_open(int32_t fd, uint8_t* buf, int32_t length);
int32_t terminal_close(int32_t fd, uint8_t* buf, int32_t length);
void update_cursor(int x, int y);


#endif /* KEYBOARD_H */
