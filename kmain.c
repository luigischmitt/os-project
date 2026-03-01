#include "io.h"
#include "framebuffer.h"
#include "serial.h"
#include "gdt.h"
#include "pic.h"
#include "idt.h"

void kmain(void) {
    serial_init(); // Call of the function that initializes the Serial Driver
    gdt_init(); // Call of the function that initializes the GDT - Memory
    pic_remap(); // Call of the function to remap the PIC
    idt_init(); // Call of the function that initializes the IDT - handler table
    
    // Interruption
    __asm__("sti");

    fb_write("GDT OK\n", 7); // Test
}