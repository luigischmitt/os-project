#ifndef INCLUDE_PIC_H
#define INCLUDE_PIC_H

/* Configures the PIC to avoid conflicts with the CPU */
void pic_remap();

/* Confirms to the hardware that the interrupt has been solved */
void pic_acknowledge(unsigned int interrupt);

#endif