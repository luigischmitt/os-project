#ifndef INCLUDE_GDT_H
#define INCLUDE_GDT_H

struct gdt_entry { // Struct to represent a segment in the table;
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char  base_middle;
    unsigned char  access;
    unsigned char  granularity;
    unsigned char  base_high;
} __attribute__((packed));

struct gdt_ptr { // Struct that contains that points towards the GDT
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

void gdt_init(void);

#endif