#include "io.h"
#include "framebuffer.h"
#include "serial.h"
#include "gdt.h"
#include "pic.h"
#include "idt.h"
#include "multiboot.h"
#include "pmm.h"
#include "vmm.h"
#include "kheap.h" /* Included the Kernel Heap */

#define VIRTUAL_KERNEL_BASE 0xC0000000
#define P_TO_V(p)  ((unsigned int)(p) + VIRTUAL_KERNEL_BASE)
#define V_TO_P(p)  ((unsigned int)(p) - VIRTUAL_KERNEL_BASE)

static void halt_forever(void)
{
    for (;;) {
        __asm__("cli; hlt");
    }
}

/* Helper function to convert an integer address to a hex string for printing */
static void uint_to_hex_str(unsigned int val, char *buffer) {
    const char *hex_chars = "0123456789ABCDEF";
    buffer[0] = '0';
    buffer[1] = 'x';
    for (int i = 7; i >= 0; i--) {
        buffer[2 + i] = hex_chars[val & 0xF];
        val >>= 4;
    }
    buffer[10] = '\0';
}

void kmain(unsigned int ebx) {

    multiboot_info_t *mbinfo = (multiboot_info_t *) (P_TO_V(ebx));
    unsigned int test_frame;
    unsigned char *temporary_page;
    char phys_str[11];
    char virt_str[11];
    
    /* Variables for the Heap test */
    char *ptr1;
    char *ptr2;
    char ptr1_str[11];
    char ptr2_str[11];

    serial_init(); /* Initialize Serial Driver */
    gdt_init();    /* Initialize GDT */
    pic_remap();   /* Remap PIC */
    idt_init();    /* Initialize IDT */

    fb_write("Kernel entered the higher half.\n", 32);
    serial_write("Kernel entered the higher half.\n", 32);

    /* Check Multiboot flags before following ebx pointer */
    if (!(mbinfo->flags & MULTIBOOT_INFO_MEM_MAP)) {
        fb_write("Error: No multiboot memory map.\n", 32);
        serial_write("Error: No multiboot memory map.\n", 32);
        halt_forever();
    }

    pmm_init(mbinfo);
    serial_write("PMM initialized.\n", 17);
    
    test_frame = pmm_alloc_frame();
    if (test_frame == 0U) {
        serial_write("Error: could not allocate a frame.\n", 35);
        halt_forever();
    }
    
    /* ==========================================================
     * TEST 1: VMM Temporary Window
     * ========================================================== */
    vmm_map_temporary(test_frame);
    temporary_page = (unsigned char *)VMM_TEMP_WINDOW;
    
    temporary_page[0] = 0xAA;
    temporary_page[PMM_FRAME_SIZE - 1U] = 0x55;
    
    vmm_map_temporary(0); /* Clean up mapping */
    serial_write("VMM temporary mapping test passed.\n", 35);

    /* ==========================================================
     * TEST 2: Persistent VMM Page Mapping
     * ========================================================== */
    unsigned int target_virtual_addr = 0xE0000000;
    
    /* Map the physical frame to our arbitrary virtual address */
    vmm_map_page(target_virtual_addr, test_frame, VMM_PRESENT | VMM_WRITABLE);

    /* Convert addresses to text */
    uint_to_hex_str(test_frame, phys_str);
    uint_to_hex_str(target_virtual_addr, virt_str);

    /* Print the results */
    serial_write("Mapped Virt Addr: ", 18);
    serial_write(virt_str, 10);
    serial_write(" to Phys Addr: ", 15);
    serial_write(phys_str, 10);
    serial_write("\n", 1);

    /* Write to the new virtual address and read it back to guarantee it works */
    unsigned int *test_ptr = (unsigned int *)target_virtual_addr;
    *test_ptr = 0xDEADBEEF;

    if (*test_ptr == 0xDEADBEEF) {
        serial_write("VMM persistent paging test successful!\n", 39);
    } else {
        serial_write("VMM persistent paging test failed!\n", 35);
    }

    /* ==========================================================
     * TEST 3: Kernel Heap Allocation (kmalloc/kfree)
     * ========================================================== */
    kheap_init();
    serial_write("Kernel Heap initialized.\n", 25);

    /* Allocate two blocks of memory */
    ptr1 = (char *)kmalloc(128);
    ptr2 = (char *)kmalloc(256);

    if (ptr1 != 0 && ptr2 != 0) {
        /* Convert their addresses to strings and print them */
        uint_to_hex_str((unsigned int)ptr1, ptr1_str);
        uint_to_hex_str((unsigned int)ptr2, ptr2_str);

        serial_write("kmalloc 1: ", 11);
        serial_write(ptr1_str, 10);
        serial_write("\n", 1);

        serial_write("kmalloc 2: ", 11);
        serial_write(ptr2_str, 10);
        serial_write("\n", 1);

        /* Write data to the heap memory to test access */
        ptr1[0] = 'H';
        ptr2[0] = 'W';

        serial_write("Heap read/write test successful!\n", 33);
        fb_write("All memory tests passed!\n", 25);
    } else {
        serial_write("Error: kmalloc returned null.\n", 30);
    }

    /* Free the memory */
    kfree(ptr1);
    kfree(ptr2);
    serial_write("Heap memory freed.\n", 19);

    halt_forever();
}