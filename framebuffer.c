#include "io.h"
#include "framebuffer.h"

#define FB_GREEN 2
#define FB_BLACK 0
/* The I/O ports */
#define FB_COMMAND_PORT         0x3D4
#define FB_DATA_PORT            0x3D5
/* The I/O port commands */
#define FB_HIGH_BYTE_COMMAND    14 // 1° turn, 
#define FB_LOW_BYTE_COMMAND     15 // 2° turn
/* The framebuffer limits */
#define FB_WIDTH  80
#define FB_HEIGHT 25


char *fb = (char *) 0x000B8000; // Pointer to the beginning of the framebuffer
static unsigned short cursor_pos = 0; // Cursor position

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
    fb[offset + 1] = ((bg & 0x0F) << 4) | (fg & 0x0F); // 2° turn. In this section, we define the foreground color and the background color
}


/** scroll:
 *  scrolls the framebuffer
 */
void scroll()
{
    unsigned int i;

    // Moves all rows up by 1 line
    for (i = 0; i < (FB_HEIGHT - 1) * FB_WIDTH; i++)
    {
        fb_write_cell(i, fb[2 * (i + FB_WIDTH)], 2, 0);
    }

    // Clears the last line on the frame buffer
    for (i = (FB_HEIGHT - 1) * FB_WIDTH; i < FB_HEIGHT * FB_WIDTH; i++)
    {
        fb_write_cell(i, ' ', 2, 0);
    }

    cursor_pos = (FB_HEIGHT - 1) * FB_WIDTH; // Updates the cursor to the last line
}


/** write:
 *  Writes a string with length len into the framebuffer
 *
 *  @param buf a pointer that points toward a string that will be written on the framebuffer
 *  @param len the length of the string
 */
int fb_write(char *buf, unsigned int len)
{
    unsigned int i;

    for (i = 0; i < len; i++) // travel around the string
    {
        char c = buf[i]; 

        if (c == '\n')
        {
            cursor_pos += FB_WIDTH - (cursor_pos % FB_WIDTH); //Move the cursor to the next line
        }
        else
        {
            fb_write_cell(cursor_pos, c, FB_GREEN, FB_BLACK); // Writes the current character
            cursor_pos++; // Increments the position
        }

        if (cursor_pos >= FB_WIDTH * FB_HEIGHT) // Verifies if the framebuffer is full
        {
            scroll(); // If it's full, then scrolls the framebuffer
        }
    }

    fb_move_cursor(cursor_pos); // Updates the frame buffer cursor

    return len;
}

void fb_decrement_cursor_pos() {
    if(cursor_pos <= 0) {
        return; // Operação inválida, pois o cursor já está no 0
    }

    cursor_pos--;
    fb_move_cursor(cursor_pos);
}