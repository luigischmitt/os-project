#ifndef INCLUDE_PIC_H
#define INCLUDE_PIC_H

/* Configura o PIC para evitar conflitos com a CPU */
void pic_remap();

/* Confirma para o hardware que a interrupção foi tratada */
void pic_acknowledge(unsigned int interrupt);

#endif