#include "file/vfs.h"
#include "file/ramfs.h"
#include "paging/kheap.h"
#include "io/serial.h"

// Dependências do Kernel
extern void* kmalloc(uint32_t size);
extern void kfree(void* ptr);

// Variáveis Globais de Estado
VFSNode* root_vnode = NULL;
VFSNode* current_vnode = NULL;

// Utilitários
static int string_compare(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

void vfs_init(void) {
    ramfs_init();

    root_vnode = (VFSNode*)kmalloc(sizeof(VFSNode));
    string_copy(root_vnode->name, "/");
    root_vnode->parent = NULL;
    root_vnode->first_child = NULL;
    root_vnode->next_sibling = NULL;

    // Cria o Inode físico raiz
    root_vnode->inode_number = ramfs_allocate_inode(DIRECTORY);

    current_vnode = root_vnode;
}

int vfs_create(const char* name, int is_directory) {
    // 1. Verifica duplicidade
    VFSNode* temp = current_vnode->first_child;
    while (temp != NULL) {
        if (string_compare(temp->name, name) == 0) {
            serial_write("Erro: Ja existe com esse nome.\n");
            return -1;
        }
        temp = temp->next_sibling;
    }

    // 2. Aloca Inode físico
    tNodeType type = is_directory ? DIRECTORY : FILE;
    uint32_t new_inode = ramfs_allocate_inode(type);
    if (new_inode == (uint32_t)-1) {
        serial_write("Erro: Tabela de Inodes cheia.\n");
        return -1;
    }

    // 3. Aloca e configura o Nó do VFS
    VFSNode* new_node = (VFSNode*)kmalloc(sizeof(VFSNode));
    string_copy(new_node->name, name);
    new_node->inode_number = new_inode;
    new_node->parent = current_vnode;
    new_node->first_child = NULL;
    new_node->next_sibling = NULL;

    // 4. Insere na árvore
    if (current_vnode->first_child == NULL) {
        current_vnode->first_child = new_node;
    } else {
        temp = current_vnode->first_child;
        while (temp->next_sibling != NULL) {
            temp = temp->next_sibling;
        }
        temp->next_sibling = new_node;
    }

    return 0;
}

int vfs_ls(const char* path) {
    (void)path;
    VFSNode* temp = current_vnode->first_child;
    
    while (temp != NULL) {
        serial_write(temp->name);
        
        tINode* inode = ramfs_get_inode(temp->inode_number);
        if (inode && inode->type == DIRECTORY) {
            serial_write("/  "); 
        } else {
            serial_write("   ");
        }
        temp = temp->next_sibling;
    }
    serial_write("\n");
    return 0;
}

int vfs_cd(const char* path) {
    if (string_compare(path, "..") == 0) {
        if (current_vnode->parent != NULL) {
            current_vnode = current_vnode->parent;
        }
        return 0;
    }

    if (string_compare(path, "/") == 0) {
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
                serial_write("Erro: Nao e diretorio.\n");
                return -1;
            }
        }
        temp = temp->next_sibling;
    }

    serial_write("Diretorio nao encontrado.\n");
    return -1;
}

int vfs_rm(const char* path) {
    VFSNode* temp = current_vnode->first_child;
    VFSNode* prev = NULL;

    while (temp != NULL) {
        if (string_compare(temp->name, path) == 0) {
            if (prev == NULL) {
                current_vnode->first_child = temp->next_sibling;
            } else {
                prev->next_sibling = temp->next_sibling;
            }

            // Opcional: Criar e chamar um ramfs_free_inode(temp->inode_number);
            kfree(temp); 
            return 0;
        }
        prev = temp;
        temp = temp->next_sibling;
    }
    
    serial_write("Nao encontrado.\n");
    return -1;
}

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

    // Sobe a árvore e empilha
    while (temp != root_vnode && depth < 16) {
        path_nodes[depth] = temp;
        temp = temp->parent;
        depth++;
    }

    // Desempilha escrevendo no buffer
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

int vfs_write(const char* path, const char* content) {
    VFSNode* temp = current_vnode->first_child;
    while (temp != NULL) {
        if (string_compare(temp->name, path) == 0) {
            return ramfs_write_file(temp->inode_number, content);
        }
        temp = temp->next_sibling;
    }
    serial_write("Arquivo nao encontrado.\n");
    return -1;
}

char* vfs_read(const char* path) {
    VFSNode* temp = current_vnode->first_child;
    while (temp != NULL) {
        if (string_compare(temp->name, path) == 0) {
            return ramfs_read_file(temp->inode_number);
        }
        temp = temp->next_sibling;
    }
    serial_write("Arquivo nao encontrado.\n");
    return NULL;
}