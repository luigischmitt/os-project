#ifndef INCLUDE_SERIAL_H
#define INCLUDE_SERIAL_H

typedef unsigned char  uint8_t;   // 1 byte 
typedef unsigned short uint16_t;  // 2 bytes 
typedef unsigned int   uint32_t;  // 4 bytes

void serial_init(void);


void serial_write_char(char c);

int serial_write(char *buf);

uint32_t string_length(const char* str);

void string_copy(char* dest, const char* src);

#endif