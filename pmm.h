#ifndef PMM_H
#define PMM_H

#include "multiboot.h"

#define PMM_FRAME_SIZE 4096U
#define PMM_TEMP_PAGE_VADDR 0xC03FF000U

void pmm_init(multiboot_info_t *mbinfo);
unsigned int pmm_alloc_frame(void);
void pmm_free_frame(unsigned int physical_address);
void *pmm_map_temporary(unsigned int physical_address);
void pmm_unmap_temporary(void);
void pmm_zero_frame(unsigned int physical_address);
unsigned int pmm_total_frames(void);
unsigned int pmm_used_frames(void);
unsigned int pmm_free_frames(void);

#endif
