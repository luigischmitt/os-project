#include "interrupt/idt.h"
#include "io/io.h"
#include "interrupt/pic.h"
#include "io/serial.h"
#include "io/framebuffer.h"
#include "interrupt/keyboard.h"
#include "shell/shell.h"

struct idt_entry idt[256]; // IDT table
struct idt_ptr idtp; // Pointer to the table

extern void interrupt_handler_0(); // Interrupt 0 - Division by 0
extern void interrupt_handler_32(); // Interrupt 32 - Timer
extern void interrupt_handler_33(); // Interrupt 33 - Keyboard
extern void load_idt(unsigned int); // Function to start loading the IDT table in the processor

void idt_set_gate(unsigned char num, unsigned int base) { // Function to set an interrupt information, base is the base address of the instruction related to the interrupt
    idt[num].offset_low = (base & 0xFFFF);
    idt[num].offset_high = (base >> 16) & 0xFFFF;
    idt[num].selector = 0x08; // GDT segment selector
    idt[num].zero = 0; // Zero
    idt[num].type_attr = 0x8E; // Present, Ring 0, 32-bit Interrupt Gate
}


void idt_init() { // Function that initialize the IDT table
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1; // IDT size
    idtp.base = (unsigned int)&idt; // IDT base address

    idt_set_gate(0, (unsigned int)interrupt_handler_0); // Puts interrupt 0 in the table
    idt_set_gate(32, (unsigned int)interrupt_handler_32); // Puts interrupt 32 in the table
    idt_set_gate(33, (unsigned int)interrupt_handler_33); // Puts the interrupt 33 in the table

    load_idt((unsigned int)&idtp); // Loads the IDT on the processor
}


/*
 * Handles hardware/CPU interrupts routed by the IDT.
 * cpu contains the saved general-purpose register snapshot.
 * interrupt is the interrupt vector number.
 * stack contains values pushed by the CPU on interrupt entry.
 */
void interrupt_handler(struct cpu_state cpu, unsigned int interrupt, struct stack_state stack) {
    (void)cpu;
    (void)stack;

    if (interrupt == 33) {
        unsigned char letter = read_letter();
        /* Routes keyboard input to the shell line editor/parser. */
        shell_handle_key((char)letter);
    }

    pic_acknowledge(interrupt); // Function to notify the hardware that the interrupt is handled
}