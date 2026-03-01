extern interrupt_handler
global load_idt

%macro no_error_code_interrupt_handler 1
    global interrupt_handler_%1
    interrupt_handler_%1:
        push    dword 0                     ; push 0 as error code
        push    dword %1                    ; push the interrupt number
        jmp     common_interrupt_handler    ; jump to the common handler
%endmacro

%macro error_code_interrupt_handler 1
    global interrupt_handler_%1
    interrupt_handler_%1:
        push    dword %1                    ; push the interrupt number
        jmp     common_interrupt_handler    ; jump to the common handler
%endmacro

common_interrupt_handler:               
    pushad                                  ; Save the registers (eax, ecx, edx, ebx, esp, ebp, esi, edi)

    call interrupt_handler                  ; Calls the C function

    popad                                   ; restore the registers

    add esp, 8                              ; restore the esp (clears the interrupt number and the error)

    iret                                    ; return to the code that got interrupted

; Handlers
no_error_code_interrupt_handler 0  ; Divide by zero
no_error_code_interrupt_handler 32 ; Timer (IRQ 0 + 32)
no_error_code_interrupt_handler 33 ; Keyboard (IRQ 1 + 32)

load_idt:
; Function to load the IDT in the processor
    mov eax, [esp + 4] 
    lidt [eax] 
    ret