#ifndef KEYBOARD_H
#define KEYBOARD_H

#define KB_PORT 0x60
#define KB_STATUS 0x64
#define KB_PRESS_MASK 0x80

void keyboard_init(void); //not sure if this is even needed

extern void keyboard_handler(void); //exception handler for keyboard


#endif /* KEYBOARD_H */
