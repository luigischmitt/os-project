#ifndef VFS_H
#define VFS_H

#include <stdint.h>

//The vfs.c will be responsible for managing the interface between the user and the ramfs.c

// Initializes the VFS, asks ramfs.c to create the root directory and defines that directory as the actual directory
void vfs_init(void);

// Navegation (The VFS manages the string with the current path)
int vfs_cd(const char* path);
void vfs_pwd(char* buffer, uint32_t max_len);

// Shell operations (The VFS manages the path and asks the ramfs.c to act)
int vfs_create(const char* path, int is_directory); // mkdir and touch
int vfs_rm(const char* path);
int vfs_ls(const char* path); 

// Write and Read
int vfs_write(const char* path, const char* data, uint32_t size);
int vfs_read(const char* path, char* buffer, uint32_t size);

#endif // VFS_H