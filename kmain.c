#include "io.h"
#include "framebuffer.h"
#include "serial.h"
#include "gdt.h"
#include "pic.h"
#include "idt.h"

void kmain(void) {
    gdt_init(); // Call of the function that initializes the GDT
    idt_init(); // Call of the function that initializes the IDT
    pic_remap(); // Call of the function to remap the PIC
    serial_init();
    
    // Interruption
    __asm__("sti");

    fb_write("GDT OK\n", 7); // Test
}