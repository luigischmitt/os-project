#include "idt.h"
#include "io.h"
#include "pic.h"
#include "serial.h"

#define KBD_DATA_PORT 0x60 // Data Port of the keyboard

struct idt_entry idt[256]; // IDT table
struct idt_ptr idtp; // Pointer to the table

extern void interrupt_handler_0(); // Interrupt 0 - Division by 0
extern void interrupt_handler_32(); // Interrupt 32 - Timer
extern void interrupt_handler_33(); // Interrupt 33 - Keyboard
extern void load_idt(unsigned int); // Function to load the IDT table in the processor

void idt_set_gate(unsigned char num, unsigned int base) { // Function to set the idt_entry information
    idt[num].offset_low = (base & 0xFFFF);
    idt[num].offset_high = (base >> 16) & 0xFFFF;
    idt[num].selector = 0x08; // GDT segment selector
    idt[num].zero = 0; // Zero
    idt[num].type_attr = 0x8E; // Present, Ring 0, 32-bit Interrupt Gate
}


unsigned char read_scan_code(void) // Function responsible for reading a scan code from the keyboard
{
    return inb(KBD_DATA_PORT);
}


void idt_init() { // Function that initialize the IDT table
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (unsigned int)&idt;

    idt_set_gate(0, (unsigned int)interrupt_handler_0); // Puts interrupt 0 in the table
    idt_set_gate(32, (unsigned int)interrupt_handler_32); // Puts interrupt 32 in the table
    idt_set_gate(33, (unsigned int)interrupt_handler_33); // Puts the interrupt 33 in the table

    load_idt((unsigned int)&idtp);
}


void interrupt_handler(struct cpu_state cpu, unsigned int interrupt, struct stack_state stack) { // Function to act if a interrupt is caught
    if (interrupt == 33) {
        unsigned char scancode = read_scan_code();

        if (scancode == 0x01) { // 0x01 is ESC scancode
            serial_write("Esc detected\n", 13);
        }

    }

    pic_acknowledge(interrupt);
}