#ifndef INCLUDE_KEYBOARD_H
#define INCLUDE_KEYBOARD_H


#define KBD_DATA_PORT 0x60 // Keyboard data port

unsigned char read_letter(void);

unsigned char read_scan_code(void);

#endif