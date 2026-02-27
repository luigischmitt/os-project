#include "idt.h"
#include "io.h"
#include "pic.h"
#include "serial.h"

#define KBD_DATA_PORT 0x60

struct idt_entry idt[256];
struct idt_ptr idtp;

extern void interrupt_handler_0();
extern void interrupt_handler_32();
extern void interrupt_handler_33();
extern void load_idt(unsigned int);

void idt_set_gate(unsigned char num, unsigned int base) {
    idt[num].offset_low = (base & 0xFFFF);
    idt[num].offset_high = (base >> 16) & 0xFFFF;
    idt[num].selector = 0x08; // Seletor de código no GDT
    idt[num].zero = 0;
    idt[num].type_attr = 0x8E; // Presente, Ring 0, 32-bit Interrupt Gate
}


unsigned char read_scan_code(void)
{
    return inb(KBD_DATA_PORT);
}


void idt_init() {
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (unsigned int)&idt;

    idt_set_gate(0, (unsigned int)interrupt_handler_0);
    idt_set_gate(32, (unsigned int)interrupt_handler_32);
    idt_set_gate(33, (unsigned int)interrupt_handler_33);

    load_idt((unsigned int)&idtp);
}


void interrupt_handler(struct cpu_state cpu, unsigned int interrupt, struct stack_state stack) {
    if (interrupt == 33) {
        unsigned char scancode = read_scan_code();

        if (scancode == 0x01) { // 0x01 is ESC scancode
            serial_write("Esc detected\n", 13);
        }

    }/* else if (interrupt == 32) {
        serial_write("Timer detectado!\n", 17);
    }*/

    pic_acknowledge(interrupt);
}