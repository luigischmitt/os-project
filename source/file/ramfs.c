#include "file/ramfs.h"
#include "paging/kheap.h"
#include "io/serial.h"

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
    return (uint32_t)-1; // Tabela cheia
}

// Locate an inode from an inode number
tINode* ramfs_get_inode(uint32_t inode_number) {
    if (inode_number >= MAX_INODES || inode_table[inode_number].used == 0) {
        return NULL;
    }
    return &(inode_table[inode_number]);
}

// Function that writes the content of the Inode and allocs the content in memory
int ramfs_write_file(uint32_t inode_number, const char* content) {
    tINode* inode = ramfs_get_inode(inode_number);
    
    // Confirms if the inode exists and if it's a file
    if (inode == NULL || inode->type != FILE) {
        return -1;
    }

    uint32_t len = string_length(content) + 1;

    // Preventing memory leak
    if (inode->data != NULL) {
        kfree(inode->data);
    }

    char* text_buffer = (char*)kmalloc(len);
    if (text_buffer == NULL) return -1; 

    string_copy(text_buffer, content);

    inode->data = text_buffer;
    inode->size = len;

    return 0; 
}

// Function that reads the content on the Inode
char* ramfs_read_file(uint32_t inode_number) {
    tINode* inode = ramfs_get_inode(inode_number);
    
    if (inode == NULL || inode->type != FILE || inode->data == NULL) {
        return NULL; 
    }

    return (char*)inode->data;
}
