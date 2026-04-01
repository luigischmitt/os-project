#include <io/utils.h>

// String functions
uint32_t string_length(const char* str) {
    uint32_t len = 0;
    while(str[len] != '\0') len++;
    return len;
}

void string_copy(char* dest, const char* src) {
    while ((*dest++ = *src++));
}

// Compare strings
int string_compare(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}