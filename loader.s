global loader                   ; the entry symbol for ELF

MAGIC_NUMBER equ 0x1BADB002
FLAGS        equ 0x00000001              ; MULTIBOOT_PAGE_ALIGN
CHECKSUM     equ -(MAGIC_NUMBER + FLAGS) ; Calculate the checksum (all options + checksum) should equal 0
KERNEL_STACK_SIZE equ 4096      ; size of stack in bytes
KERNEL_VIRTUAL_START equ 0xC0000000 ; Kernel offset

section .multiboot              ;Multiboot
align 4
    dd MAGIC_NUMBER
    dd FLAGS
    dd CHECKSUM

section .text:                  ; start of the text (code) section
align 4                         ; the code must be 4 byte aligned
    dd MAGIC_NUMBER             ; write the magic number to the machine code,
    dd FLAGS                    ; the flags,
    dd CHECKSUM                 ; and the checksum

loader:                         ; the loader label (defined as entry point in linker script)
    cli
    mov edi, ebx                ; multiboot pointer

    ; Loading the page directory in CR3
    mov eax, (boot_page_directory - 0xC0000000)
    mov cr3, eax

    ; Defining the page size to 4KB
    mov eax, cr4
    or  eax, 0x00000010
    mov cr4, eax

    ; Activing the PG bit to start pagination
    mov eax, cr0
    or  eax, 0x80000000
    mov cr0, eax

    ; Initializing higher_half
    ; Virtual address of higher_half
    lea ebx, [higher_half]
    jmp ebx ; jumps to the higher_half of the directory, eip > 0xC0000000

extern kmain  ; declare kmain as an extern function

higher_half:
    ; Now, we can remove the first entry of our directory, the identity mapping.
    mov dword [boot_page_directory - KERNEL_VIRTUAL_START], 0

    invlpg [0] ; Removes the virtual address 0 from the TLB

    mov eax, 0xCAFEBABE         ; place the number 0xCAFEBABE in the register eax
    mov esp, kernel_stack + KERNEL_STACK_SIZE ; point esp to the start of the stack (end of memory area)

    push edi                    ; Multiboot pointer
    call kmain                  ; Calls the kmain function in kmain.c

.loop:
    jmp .loop                   ; loop forever

;Funtion that will activate paging with C
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

; Function that removes a virtual address from TLB cache with C
invalidate_tlb:
    push ebp
    mov ebp, esp

    mov eax, [ebp+8] ; Gets the first argument from C, the virtual address that will be removed
    invlpg [eax] ; Execute the assembly instruction that will remove the virtual address
    
    mov esp, ebp
    pop ebp
    ret

;Function that loads the paging directory in cr3 With C
; The book recomended the creation of this function for future purposes
load_page_directory:
    push ebp
    mov ebp, esp
    mov eax, [ebp+8] ; C first argument
    mov cr3, eax ; Loads in cr3
    mov esp, ebp
    pop ebp
    ret

section .data
align 4096
boot_page_directory: ; We just need to set the directory, because we are using 4 MB pages
    dd 0x00000083 ; 0x83 = Present in memory + Read/Write + Page Size (4MB) + Identity mapping
    
    ; User memory space
    times (768 - 1) dd 0
    
    ; higher-half kernel memory space
    dd 0x00000083
    
    ; the rest of the memory spaces in the page directory
    times (1024 - 768 - 1) dd 0

section .bss
align 4                         ; align at 4 bytes
kernel_stack:                   ; label points to beginning of memory
    resb KERNEL_STACK_SIZE      ; reserve stack for the kernel


.hang:
    cli
    hlt
    jmp .hang
