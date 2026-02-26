#include "io.h"

#define FB_GREEN 2
#define FB_DARK_GREY 8
/* The I/O ports */
#define FB_COMMAND_PORT         0x3D4
#define FB_DATA_PORT            0x3D5
/* The I/O port commands */
#define FB_HIGH_BYTE_COMMAND    14 // 1° turn, 
#define FB_LOW_BYTE_COMMAND     15 // 2° turn
/* The framebuffer limits*/
#define FB_WIDTH  80
#define FB_HEIGHT 25


struct example {
    unsigned char config;      /* bit 0 - 7   */
    unsigned short address;    /* bit 8 - 23  */
    unsigned char index;       /* bit 24 - 31 */
} __attribute__((packed));     /* guarantees that the struct will have exactly 32 bits*/

char *fb = (char *) 0x000B8000; // Pointer to the beginning of the memory
static unsigned int cursor_pos = 0;

/** fb_move_cursor:
 Moves the cursor of the framebuffer to the given position
 @param pos The new position of the cursor
*/
void fb_move_cursor(unsigned short pos)
{
    outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
    outb(FB_DATA_PORT,    ((pos >> 8) & 0x00FF));
    outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
    outb(FB_DATA_PORT,    pos & 0x00FF);
}

    /** fb_write_cell:
 *  Writes a character with the given foreground and background to position i
 *  in the framebuffer.
 *
 *  @param i  The location in the framebuffer that will be written
 *  @param c  The character that will be written
 *  @param fg The foreground color
 *  @param bg The background color
 */
void fb_write_cell(unsigned int i, char c, unsigned char fg, unsigned char bg)
{
    unsigned int offset = 2 * i;
    fb[offset] = c; //1° turn. In this part we define the character of the element that we are putting in the framebuffer.
    fb[offset + 1] = ((fg & 0x0F) << 4) | (bg & 0x0F); // 2° turn. In this section, we define the foreground color and the background color
}

void scroll()
{
    unsigned int i;

    // Move todas as linhas para cima
    for (i = 0; i < (FB_HEIGHT - 1) * FB_WIDTH; i++)
    {
        fb_write_cell(i,
                      fb[2 * (i + FB_WIDTH)],
                      2,
                      0);
    }

    // Limpa última linha
    for (i = (FB_HEIGHT - 1) * FB_WIDTH;
         i < FB_HEIGHT * FB_WIDTH;
         i++)
    {
        fb_write_cell(i, ' ', 2, 0);
    }

    cursor_pos = (FB_HEIGHT - 1) * FB_WIDTH;
}

int write(char *buf, unsigned int len)
{
    unsigned int i;

    for (i = 0; i < len; i++)
    {
        char c = buf[i];

        if (c == '\n')
        {
            cursor_pos += FB_WIDTH - (cursor_pos % FB_WIDTH);
        }
        else
        {
            fb_write_cell(cursor_pos, c, 2, 0);
            cursor_pos++;
        }

        if (cursor_pos >= FB_WIDTH * FB_HEIGHT)
        {
            scroll();
        }
    }

    fb_move_cursor(cursor_pos);

    return len;
}

void kmain(void) {

    write("Hello, Kernel!\n", 15);
}