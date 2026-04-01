//The ramfs.c will be responsible for handling the iNode logic
#ifndef RAMFS_H
#define RAMFS_H

typedef unsigned char  uint8_t;   // 1 byte 
typedef unsigned short uint16_t;  // 2 bytes 
typedef unsigned int   uint32_t;  // 4 bytes

#define MAX_INODES 256 // limit of files in the file system
#define NULL (void*) 0

// Defining the types
typedef enum {
    FILE, 
    DIRECTORY
}tNodeType;

// Representation of the file in the memory ram
typedef struct iNode{
    uint32_t inode_number; // ID from 0 to 255
    tNodeType type; // FILE or DIRECTORY
    uint32_t size; // Size of the data (0 in directories)
    uint8_t used; // In use -> 1; Not in use -> 0
    
    // Will point to the text that will be allocated with kmalloc
    // If it's a directory, the vsfnode will manage that
    void* data;
} tINode;


// Functions

// Initializes the Inode table in the SO boot
void ramfs_init(void);

// Creates a Inode in memory and returns its number
uint32_t ramfs_allocate_inode(tNodeType type);

// Free inode
int ramfs_free_inode(uint32_t inode_number);

// Returns a pointer to one specific Inode from a Inode number
tINode* ramfs_get_inode(uint32_t inode_number);

// Functions to manipulate text inside a FILE
int ramfs_write_file(uint32_t inode_number, const char* content);
char* ramfs_read_file(uint32_t inode_number);

#endif // RAMFS_H