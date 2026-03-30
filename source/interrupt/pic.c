#include "io/io.h"

// PIC Ports Addresses
#define PIC1_PORT_A 0x20 
#define PIC1_PORT_B 0x21
#define PIC2_PORT_A 0xA0
#define PIC2_PORT_B 0xA1

// New addresses for the interrupts
#define PIC1_START_INTERRUPT 0x20
#define PIC2_START_INTERRUPT 0x28
#define PIC2_END_INTERRUPT PIC2_START_INTERRUPT + 7

void pic_remap() { // Remapping the PIC interrupts
    // Inicialization
    outb(PIC1_PORT_A, 0x11); 
    outb(PIC2_PORT_A, 0x11);

    // Address remap
    outb(PIC1_PORT_B, PIC1_START_INTERRUPT);    // PIC1 starts at 32
    outb(PIC2_PORT_B, PIC2_START_INTERRUPT);    // PIC2 starts at 40

    // Waterfall config
    outb(PIC1_PORT_B, 0x04);    // PIC1 sees PIC2
    outb(PIC2_PORT_B, 0x02);    // PIC2 sees PIC1

    // operation mode
    outb(PIC1_PORT_B, 0x01);    // 8086 mode
    outb(PIC2_PORT_B, 0x01);

    // Interrupt masks
    outb(PIC1_PORT_B, 0x00);
    outb(PIC2_PORT_B, 0x00);
}

#define PIC_ACK 0x20 // End of interrupt

/** pic_acknowledge:
 *  Acknowledges an interrupt from either PIC 1 or PIC 2.
 *
 *  @param interrupt The number of the interrupt
 */
void pic_acknowledge(unsigned int interrupt)
{
    if (interrupt < PIC1_START_INTERRUPT || interrupt > PIC2_END_INTERRUPT) {
        return;
    }

    if (interrupt < PIC2_START_INTERRUPT) {
        outb(PIC1_PORT_A, PIC_ACK);
    } else {
        outb(PIC2_PORT_A, PIC_ACK);
    }
}