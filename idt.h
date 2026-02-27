#ifndef INCLUDE_IDT_H
#define INCLUDE_IDT_H

struct cpu_state {
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
} __attribute__((packed));

struct stack_state {
    unsigned int error_code, eip, cs, eflags;
} __attribute__((packed));

struct idt_entry {
    unsigned short offset_low;  // The lower part of the code address            
    unsigned short selector;    // GDT Selector, probably 0x08
    unsigned char zero;    // Always zero
    unsigned char type_attr; // Flags
    unsigned short offset_high; // The other half of the code address
} __attribute__((packed));

struct idt_ptr {
    unsigned short limit; // IDT size
    unsigned int base; // IDT's base address
} __attribute__((packed));

void interrupt_handler(struct cpu_state cpu, unsigned int interrupt, struct stack_state stack);
void idt_init();

#endif