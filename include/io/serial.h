#ifndef INCLUDE_SERIAL_H
#define INCLUDE_SERIAL_H

void serial_init(void);


void serial_write_char(char c);


int  serial_write(char *buf, unsigned int len);

#endif