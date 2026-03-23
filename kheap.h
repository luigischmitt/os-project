#ifndef KHEAP_H
#define KHEAP_H

/* The magic header hidden right before each allocated memory block */
typedef struct header {
    unsigned int size;       /* Total size of the block (including this header) */
    struct header *next;     /* Pointer to the next free block (Linked List) */
} header_t;

/* Heap interface functions */
void kheap_init(void);
void *kmalloc(unsigned int size);
void kfree(void *ptr);

#endif