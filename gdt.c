#include "gdt.h"

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

struct gdt_entry gdt[3];
struct gdt_ptr gp;

extern void gdt_flush(unsigned int);

static void gdt_set_segment(int num, unsigned int base, unsigned int limit, unsigned char access, unsigned char gran)
{ // Function responsible for setting the segment fields
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = (limit >> 16) & 0x0F;

    gdt[num].granularity |= gran & 0xF0;
    gdt[num].access = access;
}

void gdt_init(void)
{ // Function responsible for initializing the segments and the GDT
    gp.limit = (sizeof(struct gdt_entry) * 3) - 1; //The size of the GDT
    gp.base = (unsigned int)&gdt; // The address of the GDT

    // Null descriptor
    gdt_set_segment(0, 0, 0, 0, 0); // index 0, base 0, limit 0, access 0, granularity 0

    // Kernel code segment
    gdt_set_segment(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // index 1, base 0, limit 0xFFFFFFFF, access 0x9A, granularity 0XCF

    // Kernel data segment
    gdt_set_segment(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // index 2, base 0, limit 0xFFFFFFFF, access 0x92, granularity 0XCF

    gdt_flush((unsigned int)&gp);
}