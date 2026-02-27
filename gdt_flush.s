global gdt_flush

gdt_flush:
    ; Gets the gdt_ptr struct pointer (from C)
    mov eax, [esp+4]

    ; Loads the GDT into the processor
    lgdt [eax]

    ; Updates the data segments (index 2 → 0x10)
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Far Jump to update CS segment selector to the code segment (index 1 → 0x08)
    jmp 0x08:flush_cs

flush_cs:
    ret