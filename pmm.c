#include "pmm.h"
#include "paging.h"

#define VIRTUAL_KERNEL_BASE         0xC0000000U
#define PMM_PAGE_PRESENT            0x1U
#define PMM_PAGE_WRITABLE           0x2U
#define PMM_LOW_MEMORY_LIMIT        0x00100000U
#define PMM_BOOTSTRAP_MAP_LIMIT     0x00400000U
#define P_TO_V(p)  ((unsigned int)(p) + VIRTUAL_KERNEL_BASE)
#define V_TO_P(p)  ((unsigned int)(p) - VIRTUAL_KERNEL_BASE)

extern unsigned int boot_page_table1[];
extern char kernel_physical_start[];
extern char kernel_physical_end[];

static unsigned int *pmm_bitmap = 0; 
static unsigned int pmm_bitmap_size = 0;
static unsigned int pmm_bitmap_physical_start = 0;
static unsigned int pmm_total_frame_count = 0;
static unsigned int pmm_used_frame_count = 0;

static unsigned int align_up(unsigned int value)
{
    return (value + PMM_FRAME_SIZE - 1U) & ~(PMM_FRAME_SIZE - 1U);
}

static unsigned int align_down(unsigned int value)
{
    return value & ~(PMM_FRAME_SIZE - 1U);
}

static int ranges_overlap(unsigned int start_a, unsigned int length_a,
                          unsigned int start_b, unsigned int length_b)
{
    unsigned int end_a;
    unsigned int end_b;

    if (length_a == 0U || length_b == 0U) {
        return 0;
    }

    end_a = start_a + length_a;
    end_b = start_b + length_b;

    return (start_a < end_b) && (start_b < end_a);
}

static int bitmap_region_overlaps_reserved(unsigned int candidate_start,
                                           unsigned int candidate_length,
                                           multiboot_info_t *mbinfo)
{
    unsigned int kernel_start = (unsigned int)kernel_physical_start;
    unsigned int kernel_end = (unsigned int)kernel_physical_end;
    unsigned int mbinfo_physical = V_TO_P(mbinfo);
    unsigned int i;

    if (ranges_overlap(candidate_start, candidate_length,
                       0x00000000U, PMM_LOW_MEMORY_LIMIT)) {
        return 1;
    }

    if (ranges_overlap(candidate_start, candidate_length,
                       kernel_start, kernel_end - kernel_start)) {
        return 1;
    }

    if (ranges_overlap(candidate_start, candidate_length,
                       mbinfo_physical, sizeof(multiboot_info_t))) {
        return 1;
    }

    if ((mbinfo->flags & MULTIBOOT_INFO_MEM_MAP) &&
        ranges_overlap(candidate_start, candidate_length,
                       mbinfo->mmap_addr, mbinfo->mmap_length)) {
        return 1;
    }

    if (mbinfo->flags & MULTIBOOT_INFO_MODS) {
        multiboot_module_t *modules = (multiboot_module_t *)(P_TO_V(mbinfo->mods_addr));
        unsigned int module_count = mbinfo->mods_count;

        if (ranges_overlap(candidate_start, candidate_length,
                           mbinfo->mods_addr,
                           module_count * sizeof(multiboot_module_t))) {
            return 1;
        }

        for (i = 0; i < module_count; i++) {
            if (ranges_overlap(candidate_start, candidate_length,
                               modules[i].mod_start,
                               modules[i].mod_end - modules[i].mod_start)) {
                return 1;
            }
        }
    }

    return 0;
}

static unsigned int clamp_range_end(multiboot_uint64_t address, multiboot_uint64_t length)
{
    multiboot_uint64_t end = address + length;

    if (end > 0x100000000ULL) {
        end = 0x100000000ULL;
    }

    return (unsigned int)end;
}

static unsigned int find_bitmap_region(multiboot_info_t *mbinfo, unsigned int bitmap_size_bytes)
{
    unsigned int mmap_current = P_TO_V(mbinfo->mmap_addr);
    unsigned int mmap_end = mmap_current + mbinfo->mmap_length;

    while (mmap_current < mmap_end) {
        multiboot_memory_map_t *entry = (multiboot_memory_map_t *)mmap_current;

        if (entry->type == MULTIBOOT_MEMORY_AVAILABLE && entry->addr < PMM_BOOTSTRAP_MAP_LIMIT) {
            unsigned int entry_start = (unsigned int)entry->addr;
            unsigned int entry_end = clamp_range_end(entry->addr, entry->len);
            unsigned int candidate;

            if (entry_end > PMM_BOOTSTRAP_MAP_LIMIT) {
                entry_end = PMM_BOOTSTRAP_MAP_LIMIT;
            }

            candidate = align_up(entry_start);
            while (candidate < entry_end) {
                if (candidate + bitmap_size_bytes > entry_end) {
                    break;
                }

                if (!bitmap_region_overlaps_reserved(candidate, bitmap_size_bytes, mbinfo)) {
                    return candidate;
                }

                candidate += PMM_FRAME_SIZE;
            }
        }

        mmap_current += entry->size + sizeof(entry->size);
    }

    return 0U;
}

static int bit_test(unsigned int frame_index)
{
    if (pmm_bitmap == 0) {
        return 1;
    }
    return pmm_bitmap[frame_index / 32U] & (1U << (frame_index % 32U));
}

static void bit_set(unsigned int frame_index)
{
    pmm_bitmap[frame_index / 32U] |= (1U << (frame_index % 32U));
}

static void bit_clear(unsigned int frame_index)
{
    pmm_bitmap[frame_index / 32U] &= ~(1U << (frame_index % 32U));
}

static void mark_frame_used(unsigned int frame_index)
{
    if (frame_index >= pmm_total_frame_count || pmm_bitmap == 0) {
        return;
    }

    if (!bit_test(frame_index)) {
        bit_set(frame_index);
        pmm_used_frame_count++;
    }
}

static void mark_frame_free(unsigned int frame_index)
{
    if (frame_index >= pmm_total_frame_count || pmm_bitmap == 0) {
        return;
    }

    if (bit_test(frame_index)) {
        bit_clear(frame_index);
        pmm_used_frame_count--;
    }
}

static void reserve_region(unsigned int start, unsigned int length)
{
    unsigned int region_start;
    unsigned int region_end;
    unsigned int address;

    if (length == 0U || pmm_bitmap == 0) {
        return;
    }

    region_start = align_down(start);
    region_end = align_up(start + length);

    for (address = region_start; address < region_end; address += PMM_FRAME_SIZE) {
        mark_frame_used(address / PMM_FRAME_SIZE);
    }
}

static void free_region(unsigned int start, unsigned int length)
{
    unsigned int region_start;
    unsigned int region_end;
    unsigned int address;

    if (length == 0U || pmm_bitmap == 0) {
        return;
    }

    region_start = align_up(start);
    region_end = align_down(start + length);

    if (region_end <= region_start) {
        return;
    }

    for (address = region_start; address < region_end; address += PMM_FRAME_SIZE) {
        mark_frame_free(address / PMM_FRAME_SIZE);
    }
}

void pmm_init(multiboot_info_t *mbinfo)
{
    unsigned int mmap_current;
    unsigned int mmap_end;
    unsigned int highest_physical_address = 0;
    unsigned int kernel_start = (unsigned int)kernel_physical_start;
    unsigned int kernel_end = (unsigned int)kernel_physical_end;
    unsigned int mbinfo_physical = V_TO_P(mbinfo);
    unsigned int i;

    pmm_bitmap = 0;
    pmm_bitmap_size = 0;
    pmm_bitmap_physical_start = 0;
    pmm_total_frame_count = 0;
    pmm_used_frame_count = 0;

    if (!(mbinfo->flags & MULTIBOOT_INFO_MEM_MAP)) {
        return;
    }

    mmap_current = P_TO_V(mbinfo->mmap_addr);
    mmap_end = mmap_current + mbinfo->mmap_length;

    while (mmap_current < mmap_end) {
        multiboot_memory_map_t *entry = (multiboot_memory_map_t *)mmap_current;
        unsigned int entry_end = clamp_range_end(entry->addr, entry->len);

        if (entry_end > highest_physical_address) {
            highest_physical_address = entry_end;
        }

        mmap_current += entry->size + sizeof(entry->size);
    }

    pmm_total_frame_count = align_up(highest_physical_address) / PMM_FRAME_SIZE;

    pmm_bitmap_size = (pmm_total_frame_count + 31U) / 32U;
    
    unsigned int bitmap_size_bytes = pmm_bitmap_size * sizeof(unsigned int);
    pmm_bitmap_physical_start = find_bitmap_region(mbinfo, bitmap_size_bytes);

    if (pmm_total_frame_count == 0U || pmm_bitmap_physical_start == 0U) {
        pmm_total_frame_count = 0;
        pmm_bitmap_size = 0;
        return;
    }

    pmm_bitmap = (unsigned int *)(P_TO_V(pmm_bitmap_physical_start));


    for (i = 0; i < pmm_bitmap_size; i++) {
        pmm_bitmap[i] = 0xFFFFFFFFU;
    }

    pmm_used_frame_count = pmm_total_frame_count;

    mmap_current = P_TO_V(mbinfo->mmap_addr);
    while (mmap_current < mmap_end) {
        multiboot_memory_map_t *entry = (multiboot_memory_map_t *)mmap_current;

        if (entry->type == MULTIBOOT_MEMORY_AVAILABLE && entry->addr < 0x100000000ULL) {
            unsigned int start = (unsigned int)entry->addr;
            unsigned int end = clamp_range_end(entry->addr, entry->len);

            if (end > start) {
                free_region(start, end - start);
            }
        }

        mmap_current += entry->size + sizeof(entry->size);
    }

    /* Keep the low memory area reserved for BIOS, GRUB and MMIO. */
    reserve_region(0x00000000U, PMM_LOW_MEMORY_LIMIT);
    reserve_region(mbinfo_physical, sizeof(multiboot_info_t));
    reserve_region(mbinfo->mmap_addr, mbinfo->mmap_length);
    reserve_region(kernel_start, kernel_end - kernel_start);
    
    /* Keep the bitmap storage itself reserved. */
    reserve_region(pmm_bitmap_physical_start, pmm_bitmap_size * sizeof(unsigned int));

    if (mbinfo->flags & MULTIBOOT_INFO_MODS) {
        multiboot_module_t *modules = (multiboot_module_t *)(P_TO_V(mbinfo->mods_addr));
        unsigned int module_count = mbinfo->mods_count;

        reserve_region(mbinfo->mods_addr, module_count * sizeof(multiboot_module_t));

        for (i = 0; i < module_count; i++) {
            reserve_region(modules[i].mod_start, modules[i].mod_end - modules[i].mod_start);
        }
    }

    boot_page_table1[1023] = 0;
    invalidate_tlb(PMM_TEMP_PAGE_VADDR);
}

unsigned int pmm_alloc_frame(void)
{
    unsigned int frame_index;

    
    for (frame_index = 0; frame_index < pmm_total_frame_count; frame_index++) {
        if (!bit_test(frame_index)) {
            mark_frame_used(frame_index);
            return frame_index * PMM_FRAME_SIZE;
        }
    }

    return 0;
}

void pmm_free_frame(unsigned int physical_address)
{
    unsigned int frame_index = align_down(physical_address) / PMM_FRAME_SIZE;
    mark_frame_free(frame_index);
}

void *pmm_map_temporary(unsigned int physical_address)
{
    unsigned int frame_base = align_down(physical_address);

    boot_page_table1[1023] = frame_base | PMM_PAGE_PRESENT | PMM_PAGE_WRITABLE;
    invalidate_tlb(PMM_TEMP_PAGE_VADDR);

    return (void *)PMM_TEMP_PAGE_VADDR;
}

void pmm_unmap_temporary(void)
{
    boot_page_table1[1023] = 0;
    invalidate_tlb(PMM_TEMP_PAGE_VADDR);
}

void pmm_zero_frame(unsigned int physical_address)
{
    unsigned char *page = (unsigned char *)pmm_map_temporary(physical_address);
    unsigned int i;

    for (i = 0; i < PMM_FRAME_SIZE; i++) {
        page[i] = 0;
    }

    pmm_unmap_temporary();
}

unsigned int pmm_total_frames(void)
{
    return pmm_total_frame_count;
}

unsigned int pmm_used_frames(void)
{
    return pmm_used_frame_count;
}

unsigned int pmm_free_frames(void)
{
    return pmm_total_frame_count - pmm_used_frame_count;
}