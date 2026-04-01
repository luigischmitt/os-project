#include "io/io.h"
#include "io/framebuffer.h"
#include "io/serial.h"
#include "segmentation/gdt.h"
#include "interrupt/pic.h"
#include "interrupt/idt.h"
#include "shell/shell.h"
#include "multiboot.h"
#include "paging/pmm.h"
#include "paging/vmm.h"
#include "paging/kheap.h"
#include "file/vfs.h"

#define VIRTUAL_KERNEL_BASE 0xC0000000
#define P_TO_V(p)  ((unsigned int)(p) + VIRTUAL_KERNEL_BASE)
#define V_TO_P(p)  ((unsigned int)(p) - VIRTUAL_KERNEL_BASE)

static void halt_forever(void)
{
    for (;;) {
        __asm__("cli; hlt");
    }
}

/*
 * Keeps the CPU in a low-power wait loop with interrupts enabled.
 * Hardware IRQs wake the CPU and execution returns to this loop after handling.
 */
static void idle_forever(void)
{
    for (;;) {
        __asm__("sti; hlt");
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

/*
 * Main kernel entry point.
 *
 * ebx contains the Multiboot information pointer passed by the bootloader.
 */
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
    /* Enables maskable interrupts so hardware IRQs can be delivered. */
    __asm__("sti");

    serial_write("Kernel entered the higher half.\n");

    /* Check Multiboot flags before following ebx pointer */
    if (!(mbinfo->flags & MULTIBOOT_INFO_MEM_MAP)) {
        serial_write("Error: No multiboot memory map.\n");
        halt_forever();
    }

    pmm_init(mbinfo);
    serial_write("PMM initialized.\n");
    
    test_frame = pmm_alloc_frame();
    if (test_frame == 0U) {
        serial_write("Error: could not allocate a frame.\n");
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
    serial_write("VMM temporary mapping test passed.\n");

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
    serial_write("Mapped Virt Addr: ");
    serial_write(virt_str);
    serial_write(" to Phys Addr: ");
    serial_write(phys_str);
    serial_write("\n");

    /* Write to the new virtual address and read it back to guarantee it works */
    unsigned int *test_ptr = (unsigned int *)target_virtual_addr;
    *test_ptr = 0xDEADBEEF;

    if (*test_ptr == 0xDEADBEEF) {
        serial_write("VMM persistent paging test successful!\n");
    } else {
        serial_write("VMM persistent paging test failed!\n");
    }

    /* ==========================================================
     * TEST 3: Kernel Heap Allocation (kmalloc/kfree)
     * ========================================================== */
    kheap_init();
    serial_write("Kernel Heap initialized.\n");

    /* Allocate two blocks of memory */
    ptr1 = (char *)kmalloc(128);
    ptr2 = (char *)kmalloc(256);

    if (ptr1 != 0 && ptr2 != 0) {
        /* Convert their addresses to strings and print them */
        uint_to_hex_str((unsigned int)ptr1, ptr1_str);
        uint_to_hex_str((unsigned int)ptr2, ptr2_str);

        serial_write("kmalloc 1: ");
        serial_write(ptr1_str);
        serial_write("\n");

        serial_write("kmalloc 2: ");
        serial_write(ptr2_str);
        serial_write("\n");

        /* Write data to the heap memory to test access */
        ptr1[0] = 'H';
        ptr2[0] = 'W';

        serial_write("Heap read/write test successful!\n");
        serial_write("All memory tests passed!\n");
    } else {
        serial_write("Error: kmalloc returned null.\n");
    }

    /* Free the memory */
    kfree(ptr1);
    kfree(ptr2);
    serial_write("Heap memory freed.\n");

    serial_write("=========================\n");
    serial_write("\n\n\n");

    /* Clears startup output before entering interactive shell mode. */
    fb_clear_screen();

    // Initializes the virtual file system
    vfs_init();

    /* Initializes shell state and prints the first prompt. */
    shell_init();

    /* Enters idle mode with interrupts enabled for interactive IRQ handling. */
    idle_forever();
}