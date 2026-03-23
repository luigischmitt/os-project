#include "io.h"
#include "framebuffer.h"
#include "serial.h"
#include "gdt.h"
#include "pic.h"
#include "idt.h"
#include "multiboot.h"
#include "pmm.h"

#define VIRTUAL_KERNEL_BASE 0xC0000000

static void halt_forever(void)
{
    for (;;) {
        __asm__("cli; hlt");
    }
}


void kmain(unsigned int ebx) {

    multiboot_info_t *mbinfo = (multiboot_info_t *) (ebx + VIRTUAL_KERNEL_BASE);
    unsigned int test_frame;
    unsigned char *temporary_page;

    serial_init(); // Call of the function that initializes the Serial Driver
    gdt_init(); // Call of the function that initializes the GDT - Memory
    pic_remap(); // Call of the function to remap the PIC
    idt_init(); // Call of the function that initializes the IDT - handler table

    fb_write("Kernel entered the higher half.\n", 31);
    serial_write("Kernel entered the higher half.\n", 31);

    // Antes de seguir o ponteiro do ebx às cegas, temos que checar as flags do struct.
    if (!(mbinfo->flags & MULTIBOOT_INFO_MODS)) {
        fb_write("Erro: GRUB nao informou modulos (flags)\n", 40);
        serial_write("Error: no multiboot memory map.\n", 31);
        halt_forever();// loop infinito para travar o kernel em caso de erro fatal
    }

    pmm_init(mbinfo);

    fb_write("PMM initialized.\n", 17);
    serial_write("PMM initialized.\n", 17);
    
    test_frame = pmm_alloc_frame();
    if (test_frame == 0U) {
        fb_write("Error: could not allocate a frame.\n", 35);
        serial_write("Error: could not allocate a frame.\n", 35);
        halt_forever();
    }
    
    
    temporary_page = (unsigned char *)pmm_map_temporary(test_frame);
    temporary_page[0] = 0xAA;
    temporary_page[PMM_FRAME_SIZE - 1U] = 0x55;
    pmm_unmap_temporary();
    pmm_free_frame(test_frame);

    fb_write("PMM temporary mapping test passed.\n", 35);
    serial_write("PMM temporary mapping test passed.\n", 35);

    halt_forever();
}