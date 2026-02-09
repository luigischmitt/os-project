// src/kernel.c
#include <stdint.h>

static void vga_print(const char *s) {
    volatile uint16_t *vga = (volatile uint16_t *)0xB8000;
    uint16_t attr = 0x0F00; // branco no preto
    for (int i = 0; s[i]; i++) {
        vga[i] = attr | (uint8_t)s[i];
    }
}

void kernel_main(void) {
    vga_print("Hello from kernel_main");
    // volta pro asm, que dá hlt em loop.
}
