#include "io.h"
#include "framebuffer.h"
#include "serial.h"
#include "gdt.h"
#include "pic.h"
#include "idt.h"
#include "multiboot.h"
#define VIRTUAL_KERNEL_BASE 0xC0000000

void kmain(unsigned int ebx) {

    multiboot_info_t *mbinfo = (multiboot_info_t *) (ebx + VIRTUAL_KERNEL_BASE);

    serial_init(); // Call of the function that initializes the Serial Driver
    gdt_init(); // Call of the function that initializes the GDT - Memory
    pic_remap(); // Call of the function to remap the PIC
    idt_init(); // Call of the function that initializes the IDT - handler table

    // Antes de seguir o ponteiro do ebx às cegas, temos que checar as flags do struct.
    if (!(mbinfo->flags & MULTIBOOT_INFO_MODS)) {
        fb_write("Erro: GRUB nao informou modulos (flags)\n", 40);
        for (;;) { __asm__("cli; hlt"); } // loop infinito para travar o kernel em caso de erro fatal
    }

    
    // Precisamos também checar se a quantidade de módulos é exatamente 1
    if (mbinfo->mods_count != 1) {
        fb_write("Erro: mods_count diferente de 1\n", 34);
        for (;;) { __asm__("cli; hlt"); } // loop infinito para travar o kernel em caso de erro fatal
    }
    
    
    // Checagem do intervalo de memória do módulo carregado, para evitar tentativas de ler o
    // código de um módulo inválido e causar comportamento indevido.
    multiboot_module_t *mods = (multiboot_module_t *) (mbinfo->mods_addr + VIRTUAL_KERNEL_BASE);
    unsigned int module_start = mods[0].mod_start + VIRTUAL_KERNEL_BASE;
    unsigned int module_end   = mods[0].mod_end + VIRTUAL_KERNEL_BASE;
    
    if (module_start >= module_end) {
        fb_write("Erro: modulo invalido\n", 22);
        for (;;) { __asm__("cli; hlt"); } // loop infinito para travar o kernel em caso de erro fatal
    }

    // Só para manter o nome presente no livro
    unsigned int address_of_module = module_start;

    // Parte que faz o "jump" para o código do programa carregado pelo GRUB,
    // tratando o endereço como um executável.
    typedef void (*call_module_t)(void); // Ponteiro para função que não recebe argumentos e retorna void.

    // Converte o valor númerico do addres_of_module para o tipo de ponteiro de função definido acima,
    // guardando na variável start_program.
    call_module_t start_program = (call_module_t) address_of_module;
    
    __asm__("cli");  // evita interrupções durante o teste

    start_program(); // em program.s entra em loop infinito
    // Não vamos passar deste ponto, somente se ocorra o retorno do código do módulo

    fb_write("GDT OK\n", 7); // Test
}