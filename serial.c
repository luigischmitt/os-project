#include "io.h"

#include "serial.h"
#include "io.h"

/* COM1 base */
#define SERIAL_COM1_BASE 0x3F8

/* Ports */
#define SERIAL_DATA_PORT(base)          (base)
#define SERIAL_FIFO_COMMAND_PORT(base)  (base + 2)
#define SERIAL_LINE_COMMAND_PORT(base)  (base + 3)
#define SERIAL_MODEM_COMMAND_PORT(base) (base + 4)
#define SERIAL_LINE_STATUS_PORT(base)   (base + 5)

/* Line config */
#define SERIAL_LINE_ENABLE_DLAB 0x80
//----------


/* Functions to configure the ports, these functions will only be utilized in this file */
static void serial_configure_baud_rate(unsigned short com, unsigned short divisor) // the function that will select the speed use for sending data
{
    outb(SERIAL_LINE_COMMAND_PORT(com), SERIAL_LINE_ENABLE_DLAB);
    outb(SERIAL_DATA_PORT(com), (divisor >> 8) & 0x00FF);
    outb(SERIAL_DATA_PORT(com), divisor & 0x00FF);
}

static void serial_configure_line(unsigned short com) // the function that will configure how data is being sent over the line
{
    /* 8 bits, no parity, 1 stop bit */
    outb(SERIAL_LINE_COMMAND_PORT(com), 0x03); // the book's standard
}

static void serial_configure_buffers(unsigned short com) // the function responsible for configuring the buffer
{
    /* Enable FIFO, clear receiver and transmission FIFO queues, 14-byte threshold */
    outb(SERIAL_FIFO_COMMAND_PORT(com), 0xC7); // Book's standard
}

static void serial_configure_modem(unsigned short com) // The function responsible for flow control of the data
{
    /* RTS = 1 and DTS = 1 */
    outb(SERIAL_MODEM_COMMAND_PORT(com), 0x03); // Book's standart
}

static int serial_is_transmit_fifo_empty(unsigned int com) // Function responsible for checking if the FIFO is empty
{
    return inb(SERIAL_LINE_STATUS_PORT(com)) & 0x20;
}
//----------------------

/* Functions that will be utilized in kmain */
void serial_init(void) // Function to initialize the serial port
{
    serial_configure_baud_rate(SERIAL_COM1_BASE, 1); // 115200 baud
    serial_configure_line(SERIAL_COM1_BASE);
    serial_configure_buffers(SERIAL_COM1_BASE);
    serial_configure_modem(SERIAL_COM1_BASE);
}

void serial_write_char(char c) // Function to write a char with the serial port
{
    while (!serial_is_transmit_fifo_empty(SERIAL_COM1_BASE));
    outb(SERIAL_DATA_PORT(SERIAL_COM1_BASE), c);
}

int serial_write(char *buf, unsigned int len) // Function to write with the serial port
{
    for (unsigned int i = 0; i < len; i++) {
        serial_write_char(buf[i]);
    }
    return len;
}