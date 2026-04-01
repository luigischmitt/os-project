#ifndef UTILS_H
#define UTILS_H

typedef unsigned char  uint8_t;   // 1 byte 
typedef unsigned short uint16_t;  // 2 bytes 
typedef unsigned int   uint32_t;  // 4 bytes

uint32_t string_length(const char* str);

void string_copy(char* dest, const char* src);

int string_compare(const char* s1, const char* s2);

#endif