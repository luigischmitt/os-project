#ifndef RAMFS_H
#define RAMFS_H
#include <stdint.h>
#define MAX_FILE_NAME 32
//The ramfs.c will be responsible for handling the iNode logic

// Defining the types
typedef char tfilename[MAX_FILE_NAME];
typedef enum {
    FILE, 
    DIRECTORY
}tNodeType;

// Representation of the file in the memory
typedef struct iNode{
    uint32_t inode_number;
    tNodeType type;
    uint32_t size;        // Size in bytes
    uint32_t link_count;  // How many names point to here
    
    // We could have direct/indirect pointers to blocks here.
    // But we're in RAM, so we just need a single pointer.
    void* data;
} tINode;

// Represents the file in a directory
typedef struct {
    tfilename name;
    uint32_t inode_number;
} tDentry;

// Functions

// Initializes the Inode table in the memory
void ramfs_init(void);

// Creates a Inode in memory and returns its number
uint32_t ramfs_allocate_inode(tNodeType type);

// Returns a pointer to one specific Inode from a Inode number
tINode* ramfs_get_inode(uint32_t inode_number);

// Adds a Dentry (name -> Inode number) to a Inode that is a directory
int ramfs_add_dentry(uint32_t dir_inode, const char* name, uint32_t target_inode);

// Reads and writes raw data in the FILE Inode void* pointer
int ramfs_write_data(uint32_t inode_number, const char* data, uint32_t size);
int ramfs_read_data(uint32_t inode_number, char* buffer, uint32_t size);

#endif // RAMFS_H