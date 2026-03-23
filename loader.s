global loader                   ; the entry symbol for ELF
global boot_page_directory
global boot_page_table1
global enable_paging
global invalidate_tlb
global load_page_directory

MAGIC_NUMBER equ 0x1BADB002
FLAGS        equ 0x00000001              ; MULTIBOOT_PAGE_ALIGN
CHECKSUM     equ -(MAGIC_NUMBER + FLAGS) ; Calculate the checksum (all options + checksum) should equal 0

KERNEL_STACK_SIZE equ 4096      ; size of stack in bytes


VIRTUAL_KERNEL_DESLOCATION equ 0xC0000000 ; Kernel offset
PAGE_FLAGS equ 0x00000003 ; Flags -> Present + Writable

section .multiboot
align 4                         ; the code must be 4 byte aligned
    dd MAGIC_NUMBER             ; write the magic number to the machine code,
    dd FLAGS                    ; the flags,
    dd CHECKSUM                 ; and the checksum


section .text               ; start of the text (code) section
loader:                         ; the loader label (defined as entry point in linker script)
    cli
    mov edi, ebx                ; multiboot pointer

    ; Initializing the boot_page_table1
    mov eax, (boot_page_table1 - VIRTUAL_KERNEL_DESLOCATION) ; Using the LMA
    mov ebx, PAGE_FLAGS         ; Frame 0 + Present + Writable
    mov ecx, 1024 ; Configuring the loop

; Function that will fill the table
.fill_table: ; Will be called in the loader because of the .
    mov [eax], ebx ; Maps the line of the table to a page frame : frame + Present + Writable
    add eax, 4 ; Next line of the table
    add ebx, 4096 ; Next page frame
    loop .fill_table ; Decrements the ecx and goes back to the start of fill table

    ; Configuring the page directory
    mov eax, (boot_page_directory - VIRTUAL_KERNEL_DESLOCATION) ; page directory lMA 
    mov ebx, (boot_page_table1 - VIRTUAL_KERNEL_DESLOCATION) ; page table 1 LMA
    or ebx, PAGE_FLAGS

    ; Identity mapping (0x0) & Higher-half kernel mapping (0xC0000000)
    mov [eax], ebx ; Identity mapping for the first 4 MB of the virtual address space.
    mov [eax + 768 * 4], ebx ; 0xC0000000 mapping to 0x0000000 and 0xC0100000 mapping to 0x0100000

    ; Loading the page directory in CR3
    mov cr3, eax

    ; Activing the PG bit to start pagination
    mov eax, cr0
    or  eax, 0x80000000 ; set the PG bit to 1
    mov cr0, eax

    ; Initializing higher_half
    ; VMA of higher_half
    lea ebx, [higher_half]
    jmp ebx ; jumps to the higher_half of the directory, eip > 0xC0000000

extern kmain  ; declare kmain as an extern function

higher_half:
    ; Now, we can remove the first entry of our directory, the identity mapping.
    mov dword [boot_page_directory], 0

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
    
    ;This part is to set the paging as true
    mov eax, cr0 ; read current cr0
    or eax, 0x80000000 ; set the PG bit to 1
    mov cr0, eax ; update cr0

    ; We don't use the CR4 here, because we aren't using 4MB page frames.
    
    mov esp, ebp
    pop ebp
    ret

; Function that removes a VMA from TLB cache with C
invalidate_tlb:
    push ebp
    mov ebp, esp

    mov eax, [ebp+8] ; Gets the first argument from C, the VMA that will be removed
    invlpg [eax] ; Execute the assembly instruction that will remove the VMA from the TLB
    
    mov esp, ebp
    pop ebp
    ret

;Function that loads the paging directory in cr3 With C
; The book recommended the creation of this function for future purposes
load_page_directory:
    push ebp
    mov ebp, esp
    mov eax, [ebp+8] ; C first argument
    mov cr3, eax ; Loads in cr3
    mov esp, ebp
    pop ebp
    ret


section .bss
align 4096
boot_page_directory:
    resb 4096 ; Reserve the boot_page_directory
boot_page_table1:
    resb 4096 ; Reserve the boot_page_table1
kernel_stack:                   ; label points to beginning of memory
    resb KERNEL_STACK_SIZE      ; reserve stack for the kernel

.hang:
    cli
    hlt
    jmp .hang