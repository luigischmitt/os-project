#include "interrupt/keyboard.h"
#include "io/io.h"

/* Number of entries available in the scan-code translation table. */
#define KEYBOARD_MAP_SIZE 128U

/* Keyboard mapping */
static const unsigned char keyboard_map[KEYBOARD_MAP_SIZE] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', 
  '9', '0', '-', '=', '\b', 
  '\t',                     
  'q', 'w', 'e', 'r',       
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 
    0,                      
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', 
 '\'', '`',   0,            
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',                
  'm', ',', '.', '/',   0,  
  '*',
    0,  
  ' ',  
    0   
    /* The rest will be initialized with 0 */
};

/**
 * Converts the latest keyboard scan code into an ASCII character.
 *
 * Returns '\0' for key-release events, unmapped keys, or invalid scan codes.
 */
unsigned char read_letter(void)
{
  unsigned char scan_code = read_scan_code();

  if ((scan_code & 0x80U) != 0U) {
    return '\0';
  }

  if (scan_code >= KEYBOARD_MAP_SIZE) {
    return '\0';
  }

  return keyboard_map[scan_code];
}

/**
 * Reads one raw scan code byte from the PS/2 keyboard data port.
 */
unsigned char read_scan_code(void)
{
  return inb(KBD_DATA_PORT);
}