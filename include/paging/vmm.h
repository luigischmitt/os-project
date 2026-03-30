#ifndef VMM_H
#define VMM_H

/* Magic address of our Temporary Window */
#define VMM_TEMP_WINDOW 0xC03FF000

/* Configuration bits for PDE and PTE entries */
#define VMM_PRESENT  0x1  /* Bit P: Page is in RAM */
#define VMM_WRITABLE 0x2  /* Bit R/W: 1 = Read/Write, 0 = Read-Only */
#define VMM_USER     0x4  /* Bit U/S: 1 = User mode, 0 = Supervisor (Kernel) */

/* External assembly function to flush the TLB cache */
extern void invalidate_tlb(unsigned int virtual_address);

/**
 * Maps a virtual address to a physical address
 * @param virtual_addr The virtual address to be used by the software
 * @param physical_addr The real physical address from PMM
 * @param flags Permissions and status bits (VMM_PRESENT, etc)
 */
void vmm_map_page(unsigned int virtual_addr, unsigned int physical_addr, unsigned int flags);

/**
 * Maps the temporary window to a physical address
 */
void vmm_map_temporary(unsigned int physical_address);

#endif