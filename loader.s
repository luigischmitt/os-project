global loader                   ; the entry symbol for ELF

MAGIC_NUMBER equ 0x1BADB002
FLAGS        equ 0x00000001              ; MULTIBOOT_PAGE_ALIGN
CHECKSUM     equ -(MAGIC_NUMBER + FLAGS) ; Calculate the checksum (all options + checksum) should equal 0

section .multiboot              ;Multiboot
align 4
    dd MAGIC_NUMBER
    dd FLAGS
    dd CHECKSUM

KERNEL_STACK_SIZE equ 4096      ; size of stack in bytes

section .bss
align 4                         ; align at 4 bytes
kernel_stack:                   ; label points to beginning of memory
    resb KERNEL_STACK_SIZE      ; reserve stack for the kernel

section .text:                  ; start of the text (code) section
align 4                         ; the code must be 4 byte aligned
    dd MAGIC_NUMBER             ; write the magic number to the machine code,
    dd FLAGS                    ; the flags,
    dd CHECKSUM                 ; and the checksum

extern kmain           ; declare kmain as an extern function

loader:                         ; the loader label (defined as entry point in linker script)
    mov eax, 0xCAFEBABE         ; place the number 0xCAFEBABE in the register eax
    
    mov esp, kernel_stack + KERNEL_STACK_SIZE ; point esp to the start of the stack (end of memory area)


    call kmain                  ; Calls the kmain function in kmain.c

.loop:
    jmp .loop                   ; loop forever

;Funtion that will activate paging
enable_paging:
    push ebp    
    mov ebp, esp
    
    ;This part is responsible for setting the page frame size to 4MB
    mov eax, cr4 ; read current cr4
    or eax, 0x00000010 ; Set the PSE bit
    mov cr4, eax ; update cr4

    ;This part is to set the paging as true
    mov eax, cr0 ; read current cr0
    or eax, 0x80000000 ; set the PG bit to 1
    mov cr0, eax ; update cr0

    
    mov esp, ebp
    pop ebp
    ret

; Function that removes a virtual address from TLB cache
invalidate_tlb:
    push ebp
    mov ebp, esp

    mov eax, [ebp+8] ; Gets the first argument from C, the virtual address that will be removed
    invlpg [eax] ; Execute the assembly instruction that will remove the virtual address
    
    mov esp, ebp
    pop ebp
    ret

;Function that loads the paging directory in cr3
; The book recomended the creation of this function for future purposes
load_page_directory:
    push ebp
    mov ebp, esp
    mov eax, [ebp+8] ; C first argument
    mov cr3, eax ; Loads in cr3
    mov esp, ebp
    pop ebp
    ret

.hang:
    cli
    hlt
    jmp .hang
