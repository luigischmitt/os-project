//The vfs.c will be responsible for managing the interface between the user and the ramfs.c
#ifndef VFS_H
#define VFS_H

typedef unsigned char  uint8_t;   // 1 byte  (8 bits)
typedef unsigned short uint16_t;  // 2 bytes (16 bits)
typedef unsigned int   uint32_t;  // 4 bytes (32 bits)

#define MAX_FILE_NAME 32 // Max name size
#define NULL (void*) 0

// Defining the types
typedef char tfilename[MAX_FILE_NAME];

// VFS node
typedef struct VFSNode {
    tfilename name;
    uint32_t inode_number; // The connection between the inode and VFSnode
    
    // Pointers to navigation
    struct VFSNode* parent;
    struct VFSNode* first_child; // If it's a directory
    struct VFSNode* next_sibling;
} VFSNode;

// Initializes the VFS, asks ramfs.c to create the root directory and defines that directory as the actual directory
void vfs_init(void);

// Navigation (The VFS manages the string with the current path)
int vfs_cd(const char* path);
void vfs_pwd(char* buffer, uint32_t max_len);

// Shell operations (The VFS manages the path and asks the ramfs.c to act)
int vfs_create(const char* path, int is_directory); // mkdir and touch
int vfs_rm(const char* path);
int vfs_ls(const char* path);


// Read and write
int vfs_write(const char* path, const char* content);
char* vfs_read(const char* path);

// Freeing of the nodes
void vfs_free_all(void);

#endif // VFS_H