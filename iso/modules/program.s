BITS 32

; Mensagem curta no canto superior esquerdo da tela do Bochs.
; Cada caractere em text mode ocupa 2 bytes: [char][atributo].
mov edi, 0xC00B8000
mov ah, 0x1F                 ; branco sobre azul

mov al, 'M'  ; "MOD LOOP"
stosw
mov al, 'O'
stosw
mov al, 'D'
stosw
mov al, ' '
stosw
mov al, 'L'
stosw
mov al, 'O'
stosw
mov al, 'O'
stosw
mov al, 'P'
stosw

; Valor de teste pedido no livro/log
mov eax, 0xDEADBEEF

; Loop infinito do modulo
jmp $