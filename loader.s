global loader
global boot_page_directory
global boot_page_table1
global enable_paging
global invalidate_tlb
global load_page_directory

MAGIC_NUMBER equ 0x1BADB002
FLAGS        equ 0x00000003              ; MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO
CHECKSUM     equ -(MAGIC_NUMBER + FLAGS) ; (magic + flags + checksum) must equal 0

KERNEL_STACK_SIZE equ 4096

VIRTUAL_KERNEL_DESLOCATION equ 0xC0000000
PAGE_FLAGS equ 0x00000003                ; Present + Writable

section .multiboot
align 4
    dd MAGIC_NUMBER
    dd FLAGS
    dd CHECKSUM

section .text
loader:
    cli
    mov edi, ebx

    ; Fill the first page table and reserve its last entry for temporary mappings.
    mov eax, (boot_page_table1 - VIRTUAL_KERNEL_DESLOCATION)
    mov ebx, PAGE_FLAGS
    mov ecx, 1023

.fill_table:
    mov [eax], ebx
    add eax, 4
    add ebx, 4096
    loop .fill_table

    mov dword [eax], 0

    ; Map the first page table both low and in the higher half.
    mov eax, (boot_page_directory - VIRTUAL_KERNEL_DESLOCATION)
    mov ebx, (boot_page_table1 - VIRTUAL_KERNEL_DESLOCATION)
    or ebx, PAGE_FLAGS

    mov [eax], ebx
    mov [eax + 768 * 4], ebx

    mov cr3, eax

    mov eax, cr0
    or  eax, 0x80000000
    mov cr0, eax

    lea ebx, [higher_half]
    jmp ebx

extern kmain

higher_half:
    mov dword [boot_page_directory], 0
    invlpg [0]

    mov eax, 0xCAFEBABE
    mov esp, kernel_stack + KERNEL_STACK_SIZE

    push edi
    call kmain

.loop:
    jmp .loop

enable_paging:
    push ebp
    mov ebp, esp

    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    mov esp, ebp
    pop ebp
    ret

invalidate_tlb:
    push ebp
    mov ebp, esp

    mov eax, [ebp + 8]
    invlpg [eax]

    mov esp, ebp
    pop ebp
    ret

load_page_directory:
    push ebp
    mov ebp, esp

    mov eax, [ebp + 8]
    mov cr3, eax

    mov esp, ebp
    pop ebp
    ret

section .bss
align 4096
boot_page_directory:
    resb 4096
boot_page_table1:
    resb 4096
kernel_stack:
    resb KERNEL_STACK_SIZE

.hang:
    cli
    hlt
    jmp .hang
