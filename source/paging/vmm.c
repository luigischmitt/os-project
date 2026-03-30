#include "paging/vmm.h"
#include "paging/pmm.h"

/* Import the original Page Directory and Table created in loader.s */
extern unsigned int boot_page_directory[1024];
extern unsigned int boot_page_table1[1024];

void vmm_map_temporary(unsigned int physical_address) {
    /* Write physical address to the last entry of the Kernel Table (Index 1023) */
    boot_page_table1[1023] = (physical_address & 0xFFFFF000) | VMM_PRESENT | VMM_WRITABLE;

    /* Invalidate TLB cache so CPU recognizes the change immediately */
    invalidate_tlb(VMM_TEMP_WINDOW);
}

void vmm_map_page(unsigned int virtual_addr, unsigned int physical_addr, unsigned int flags) {
    /* 1. Extract indexes from the virtual address */
    unsigned int pd_index = virtual_addr >> 22;
    unsigned int pt_index = (virtual_addr >> 12) & 0x03FF;

    /* 2. Check if the Page Table exists in the Directory */
    if ((boot_page_directory[pd_index] & VMM_PRESENT) != VMM_PRESENT) {
        
        /* Table doesn't exist, allocate a new frame for it */
        unsigned int new_table_physical = pmm_alloc_frame();

        if (new_table_physical == 0) {
            return; /* Out of Memory */
        }

        /* Use the temporary window to access and clear the new table */
        vmm_map_temporary(new_table_physical);
        unsigned int *temp_table = (unsigned int *) VMM_TEMP_WINDOW;

        for (int i = 0; i < 1024; i++) {
            temp_table[i] = 0;
        }

        /* Connect the new table to the Page Directory */
        boot_page_directory[pd_index] = new_table_physical | VMM_PRESENT | VMM_WRITABLE | VMM_USER;
    }

    /* 3. Final mapping in the Page Table (PTE) */
    unsigned int table_physical = boot_page_directory[pd_index] & 0xFFFFF000;

    /* Point the window to the target table */
    vmm_map_temporary(table_physical);
    unsigned int *pt = (unsigned int *) VMM_TEMP_WINDOW;

    /* Insert the Physical Address + Flags */
    pt[pt_index] = (physical_addr & 0xFFFFF000) | (flags & 0xFFF) | VMM_PRESENT;

    /* Flush TLB for the specific virtual address */
    invalidate_tlb(virtual_addr);
}