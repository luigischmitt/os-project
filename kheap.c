#include "kheap.h"
#include "pmm.h"
#include "vmm.h"

/* Virtual Address where our Heap will start (Arbitrary kernel space address) */
#define KHEAP_START 0xD0000000
#define KHEAP_INITIAL_SIZE 4096 /* We start with 1 page (4 KB) */

/* Our Linked List of free blocks */
static header_t *free_list = 0;

/* Tracks the current end of the mapped heap for future expansions */
static unsigned int heap_current_top = KHEAP_START;

void kheap_init(void) {
    /* 1. Request our first 4 KB frame from the physical memory manager (PMM) */
    unsigned int phys_addr = pmm_alloc_frame();

    /* 2. Build the virtual bridge for it using the VMM */
    /* We grant Present and Writable permissions for the Kernel */
    vmm_map_page(KHEAP_START, phys_addr, VMM_PRESENT | VMM_WRITABLE);

    /* Update the boundary tracker */
    heap_current_top += KHEAP_INITIAL_SIZE;

    /* 3. Take this entire page and turn it into our first giant free block! */
    free_list = (header_t *) KHEAP_START;

    /* The block size is the full 4096 bytes */
    free_list->size = KHEAP_INITIAL_SIZE;

    /* Since it's the only free block, there is no next block */
    free_list->next = 0;
}

void *kmalloc(unsigned int size) {
    if (size == 0) return 0;

    /* 1. Actual size includes what the user requested + our hidden header */
    unsigned int total_size = size + sizeof(header_t);

    header_t *current = free_list;
    header_t *previous = 0;

    /* 2. Sweep the free block list looking for one that fits */
    while (current != 0) {
        if (current->size >= total_size) {
            /* WE FOUND A BLOCK! */

            /* 3. Is the block large enough to be split in two? */
            /* We only split if the remainder is larger than a header */
            if (current->size > total_size + sizeof(header_t)) {

                /* The math of the cut: The new free block starts right after the block we will return */
                /* We use (char*) to make C jump the exact amount of bytes */
                header_t *new_free_block = (header_t *) ((char *)current + total_size);

                new_free_block->size = current->size - total_size;
                new_free_block->next = current->next;

                /* Update the current block to the exact required size */
                current->size = total_size;
                current->next = new_free_block;
            }

            /* 4. Remove the 'current' block from the free list */
            if (previous == 0) {
                /* It was the first in the list */
                free_list = current->next;
            } else {
                /* It was in the middle of the list */
                previous->next = current->next;
            }

            /* 5. THE GREAT TRICK: We return the pointer immediately AFTER the header */
            /* Hiding the metadata from the programmer */
            return (void *) ((char *)current + sizeof(header_t));
        }

        previous = current;
        current = current->next;
    }

    /* 6. WHAT IF MEMORY RUNS OUT? (The sbrk/brk equivalent) */
    /* Following the book's instruction to replace sbrk with the page frame allocator. */
    unsigned int phys_addr = pmm_alloc_frame();
    
    if (phys_addr == 0) {
        return 0; /* Out of physical memory (OOM) */
    }

    /* Map the new physical frame to the current end of the heap */
    vmm_map_page(heap_current_top, phys_addr, VMM_PRESENT | VMM_WRITABLE);
    
    /* Turn this new page into a free block */
    header_t *new_expansion = (header_t *) heap_current_top;
    new_expansion->size = PMM_FRAME_SIZE;
    
    /* Insert it at the beginning of the free list */
    new_expansion->next = free_list;
    free_list = new_expansion;

    /* Update the heap boundary */
    heap_current_top += PMM_FRAME_SIZE;

    /* Retry the allocation recursively now that we have more memory */
    return kmalloc(size);
}

void kfree(void *ptr) {
    if (ptr == 0) return; /* Safety against null pointers */

    /* 1. Reverse Math: Go back the exact bytes of the header size */
    /* to find the original metadata that kmalloc hid. */
    header_t *block = (header_t *) ((char *)ptr - sizeof(header_t));

    /* 2. Insert the block back at the BEGINNING of the free block list */
    block->next = free_list;
    free_list = block;
}