#include "file/vfs.h"
#include "file/ramfs.h"
#include "paging/kheap.h"
#include "io/serial.h"
#include "io/utils.h"
#include "io/framebuffer.h"

// Global variables
VFSNode* root_vnode = NULL; // Virtual node root
VFSNode* current_vnode = NULL; // current node

// Initialization of virtual file system
void vfs_init(void) {
    ramfs_init(); // Initialization of the inode table

    root_vnode = (VFSNode*)kmalloc(sizeof(VFSNode)); // allocation of the root node
    if(!root_vnode) return -1;
    string_copy(root_vnode->name, "/");
    root_vnode->parent = NULL;
    root_vnode->first_child = NULL;
    root_vnode->next_sibling = NULL;

    // Creates the inode for the root
    root_vnode->inode_number = ramfs_allocate_inode(DIRECTORY);

    current_vnode = root_vnode; // Updates the current node
}

// Creating a virtual file system node
int vfs_create(const char* name, int is_directory) {
    // Checks for duplicity
    VFSNode* temp = current_vnode->first_child;
    while (temp != NULL) {
        if (string_compare(temp->name, name) == 0) {
            fb_write("Erro: Ja existe um arquivo com esse nome.\n");
            return -1;
        }
        temp = temp->next_sibling;
    }

    // Allocating inode for the vfsnode
    tNodeType type = is_directory ? DIRECTORY : FILE;
    uint32_t new_inode = ramfs_allocate_inode(type);
    if (new_inode == (uint32_t)-1) {
        fb_write("Erro: Tabela de Inodes cheia.\n");
        return -1;
    }

    //Allocating and managing vfsnode
    VFSNode* new_node = (VFSNode*)kmalloc(sizeof(VFSNode));
    if(!new_node) return -1;

    string_copy(new_node->name, name);
    new_node->inode_number = new_inode;
    new_node->parent = current_vnode;
    new_node->first_child = NULL;
    new_node->next_sibling = NULL;

    // Inserting in the tree
    if (current_vnode->first_child == NULL) {
        current_vnode->first_child = new_node; // If it's the first child
    } else {
        temp = current_vnode->first_child;
        while (temp->next_sibling != NULL) {
            temp = temp->next_sibling;
        }
        temp->next_sibling = new_node;
    }
    return 0;
}

// Function responsible for listing all the children of a directory
int vfs_ls(const char* path) {
    (void)path;
    VFSNode* temp = current_vnode->first_child;
    
    while (temp != NULL) {
        fb_write(temp->name);
        
        tINode* inode = ramfs_get_inode(temp->inode_number);
        if (inode && inode->type == DIRECTORY) {
            fb_write("/  "); 
        } else {
            fb_write("   ");
        }
        temp = temp->next_sibling;
    }
    fb_write("\n");
    return 0;
}

// Function responsible for navigation between directories
int vfs_cd(const char* path) {
    if (string_compare(path, "..") == 0) { // Parent
        if (current_vnode->parent != NULL) {
            current_vnode = current_vnode->parent;
        }
        return 0;
    }

    if (string_compare(path, "/") == 0) { // Root
        current_vnode = root_vnode;
        return 0;
    }

    VFSNode* temp = current_vnode->first_child;
    while (temp != NULL) {
        if (string_compare(temp->name, path) == 0) {
            tINode* inode = ramfs_get_inode(temp->inode_number);
            if (inode != NULL && inode->type == DIRECTORY) {
                current_vnode = temp;
                return 0;
            } else {
                fb_write("Erro: Nao e diretorio.\n");
                return -1;
            }
        }
        temp = temp->next_sibling;
    }

    fb_write("Diretorio nao encontrado.\n");
    return -1;
}

// Function that deletes a file
int vfs_rm(const char* path) {
    VFSNode* temp = current_vnode->first_child;
    VFSNode* prev = NULL;

    while (temp != NULL) {
        if (string_compare(temp->name, path) == 0) {
            if (temp->first_child != NULL) {
                fb_write("Erro: Diretorio nao esta vazio. Apague os arquivos primeiro.\n");
                return -1;
            }

            if (prev == NULL) {
                current_vnode->first_child = temp->next_sibling;
            } else {
                prev->next_sibling = temp->next_sibling;
            }

            ramfs_free_inode(temp->inode_number);
            kfree(temp); 
            return 0;
        }
        prev = temp;
        temp = temp->next_sibling;
    }
    
    fb_write("Nao encontrado.\n");
    return -1;
}

// Function that changes the buffer argument with the pwd
void vfs_pwd(char* buffer, uint32_t max_len) {
    // Initial directory
    if (current_vnode == root_vnode) {
        string_copy(buffer, "/");
        return;
    }

    buffer[0] = '\0';
    VFSNode* temp = current_vnode;
    VFSNode* path_nodes[16]; 
    int depth = 0;

    // climbs the tree and starts stacking
    while (temp != root_vnode && depth < 16) {
        path_nodes[depth] = temp;
        temp = temp->parent;
        depth++;
    }

    // unstacks writing on the buffer
    uint32_t buf_idx = 0;
    for (int i = depth - 1; i >= 0; i--) {
        if (buf_idx < max_len - 1) {
            buffer[buf_idx++] = '/';
            char* name = path_nodes[i]->name;
            int j = 0;
            while (name[j] != '\0' && buf_idx < max_len - 1) {
                buffer[buf_idx++] = name[j++];
            }
        }
    }
    buffer[buf_idx] = '\0';
}

// ==========================================
// Read / Write functions
// ==========================================

// Function that starts the process of writing on a file
int vfs_write(const char* path, const char* content) {
    VFSNode* temp = current_vnode->first_child;
    while (temp != NULL) {
        if (string_compare(temp->name, path) == 0) {
            return ramfs_write_file(temp->inode_number, content); // Goes to the function to write on a inode
        }
        temp = temp->next_sibling;
    }
    serial_write("Arquivo nao encontrado.\n");
    return -1;
}

// Function that starts the process of reading a file
char* vfs_read(const char* path) {
    VFSNode* temp = current_vnode->first_child;
    while (temp != NULL) {
        if (string_compare(temp->name, path) == 0) {
            return ramfs_read_file(temp->inode_number); // Goes to the function that reads an Inode
        }
        temp = temp->next_sibling;
    }
    serial_write("Arquivo nao encontrado.\n");
    return NULL;
}