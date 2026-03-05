global loader

MAGIC_NUMBER equ 0x1BADB002
FLAGS        equ 0x00000001            ; MULTIBOOT_PAGE_ALIGN
CHECKSUM     equ -(MAGIC_NUMBER + FLAGS)

KERNEL_STACK_SIZE equ 4096

section .multiboot
align 4
    dd MAGIC_NUMBER
    dd FLAGS
    dd CHECKSUM

section .bss
align 16
kernel_stack:
    resb KERNEL_STACK_SIZE

section .text
align 4
extern kmain

loader:
    mov esp, kernel_stack + KERNEL_STACK_SIZE
    push ebx
    call kmain

.hang:
    cli
    hlt
    jmp .hang