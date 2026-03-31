#ifndef VFS_H
#define VFS_H
#define MAX_FILE_NAME 32

typedef char tfilename[MAX_FILE_NAME];
typedef enum {
    FILE, 
    DIRECTORY
}tNodeType;


typedef struct VFSNode{
    tfilename name;
    unsigned int size;
    tNodeType type;

    struct VFSNode* parent;
    struct VFSNode* next_sibling;

    union {
        // type == DIRECTORY
        // Pointer to the first file in a directory
        // Will be utilized to iterate in a directory
        struct VFSNode* first_child;

        // type == FILE
        // Pointer to the data
        char* data;
    } content;
} tVFSNode;

// Global variable to control where the shell is at the moment
extern tVFSNode* current_directory;
extern tVFSNode* root_directory;

// ==========================================
// API Prototype
// ==========================================

// Starts the file system creating the root "/"
void vfs_init();

// Function to alloc memory creating a new node
tVFSNode* vfs_create_node(const char* name, tNodeType type, tVFSNode* parent);

// Searchs for a file or directory on a base directory
tVFSNode* vfs_find_node(tVFSNode* start_dir, const char* name);

// Frees the node's memory
void vfs_delete_node(tVFSNode* node);

// Functions that will manipulate the files
int vfs_read(tVFSNode* file, char* buffer, unsigned int size);
int vfs_write(tVFSNode* file, const char* buffer, unsigned int size);

#endif // VFS_H