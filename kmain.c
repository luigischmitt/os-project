#include "io.h"
#include "framebuffer.h"
#include "serial.h"
#include "gdt.h"

void kmain(void) {
    gdt_init(); // Call of the function that initializes the GDT

    fb_write("GDT OK\n", 7); // Test
    serial_init();
    serial_write("Line2\n", 6);
}