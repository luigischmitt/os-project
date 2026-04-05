#include "file/ramfs.h"
#include "paging/kheap.h"
#include "io/utils.h"

// Inodes table
tINode inode_table[MAX_INODES];

// Table initialization
void ramfs_init(void) {
    for (int i = 0; i < MAX_INODES; i++) {
        inode_table[i].used = 0;
        inode_table[i].size = 0;
        inode_table[i].data = NULL; 
    }
}

// Inode allocation
uint32_t ramfs_allocate_inode(tNodeType type) {
    for (uint32_t i = 0; i < MAX_INODES; i++) {
        if (inode_table[i].used == 0) {
            inode_table[i].used = 1;
            inode_table[i].inode_number = i;
            inode_table[i].type = type;
            inode_table[i].size = 0;
            inode_table[i].data = NULL;
            return i; // returns the index to the vfsnode
        }
    }
    return (uint32_t)-1; // Indicates that the table is full
}

// Inode free
int ramfs_free_inode(uint32_t inode_number) {
    tINode* inode = ramfs_get_inode(inode_number);
    
    // Verifies if the inode exists and is in use
    if (inode == NULL) {
        return -1; 
    }

    // If it is a file and has allocated text, free the memory to prevent leaks
    if (inode->type == FILE && inode->data != NULL) {
        kfree(inode->data);
    }

    // Reset the inode fields so it becomes available again
    inode->used = 0;
    inode->size = 0;
    inode->data = NULL;

    return 0;
}

// Locate an inode with an inode number
tINode* ramfs_get_inode(uint32_t inode_number) {
    if (inode_number >= MAX_INODES || inode_table[inode_number].used == 0) {
        return NULL;
    }
    return &(inode_table[inode_number]);
}

// Function that writes the content of the Inode and allocs that content in memory
int ramfs_write_file(uint32_t inode_number, const char* content) {
    tINode* inode = ramfs_get_inode(inode_number); // Getting the inode
    
    // Confirms if the inode exists and if it's not a file
    if (inode == NULL || inode->type != FILE) {
        return -1;
    }

    uint32_t len = string_length(content) + 1; // Length of the content of the Inode

    // Preventing memory leak
    if (inode->data != NULL) {
        kfree(inode->data);
    }

    char* text_buffer = (char*)kmalloc(len); //Allocating the content in virtual memory
    if (text_buffer == NULL) return -1; 

    string_copy(text_buffer, content); // Writing the content in that space in virtual memory

    inode->data = text_buffer; // putting the content into the data section of the inode
    inode->size = len; // Setting the size of the content in the inode

    return 0; 
}

// Function that reads the content on the Inode
char* ramfs_read_file(uint32_t inode_number) {
    tINode* inode = ramfs_get_inode(inode_number);
    
    // Verifies if the inode exists, if it's a file and if it has data
    if (inode == NULL || inode->type != FILE || inode->data == NULL) {
        return NULL; 
    }

    return (char*)inode->data; // Returns the content of the Inode
}
