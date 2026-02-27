#include "keyboard.h"
#include "io.h"

/* Keyboard mapping */
unsigned char keyboard_map[128] = {
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

unsigned char read_letter(void) // Function responsible for reading a letter from the keyboard
{
    return keyboard_map[read_scan_code()];
}

unsigned char read_scan_code(void) // Function responsible for reading a scan code from the keyboard
{
  return inb(KBD_DATA_PORT);
}