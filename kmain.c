#include "io.h"
#include "framebuffer.h"
#include "serial.h"

void kmain(void) {

    fb_write("Hello, Kernel!\n", 15); // Test
    serial_init();
    serial_write("Line\n", 5);
}