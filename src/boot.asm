; src/boot.asm
BITS 32

; --- Multiboot header (GRUB procura isso no começo do ELF) ---
SECTION .multiboot
align 4
    dd 0x1BADB002            ; magic
    dd 0x00000000            ; flags (0 = mínimo)
    dd -(0x1BADB002 + 0x00000000) ; checksum

; --- Stack para o kernel ---
KERNEL_STACK_SIZE equ 4096                  ; tamanho da stack em bytes

SECTION .bss
align 4                                      ; alinhamento de 4 bytes
kernel_stack:                                ; label aponta para o início da memória reservada
    resb KERNEL_STACK_SIZE                   ; reserva stack para o kernel

SECTION .text
global _start

_start:
    ; Inicializa stack do kernel
    mov esp, kernel_stack + KERNEL_STACK_SIZE  ; aponta ESP para o topo da stack

    ; "Hello Cafebabe": coloca 0xCAFEBABE no EAX
    mov eax, 0xCAFEBABE

.hang:
    cli
    hlt
    jmp .hang
