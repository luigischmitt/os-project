; src/boot.asm
BITS 32

; --- Multiboot header (GRUB procura isso no começo do ELF) ---
SECTION .multiboot
align 4
    dd 0x1BADB002            ; magic
    dd 0x00000000            ; flags (0 = mínimo)
    dd -(0x1BADB002 + 0x00000000) ; checksum

SECTION .text
global _start
extern kernel_main

_start:
    ; "Hello Cafebabe": coloca 0xCAFEBABE no EAX e trava.
    mov eax, 0xCAFEBABE

    ; chama o kernel em C (só pra provar o pipeline)
    call kernel_main

.hang:
    cli
    hlt
    jmp .hang
