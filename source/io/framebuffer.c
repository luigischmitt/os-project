#include "io/io.h"
#include "io/framebuffer.h"

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

char *fb = (char *) 0xC00B8000; // Pointer to the beginning of the framebuffer, now considering the 3GB headstart of the kernel in the virtual memory.
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
 * @param i Linear cell index in the framebuffer.
 * @param c Character to be written.
 * @param fg Foreground color (low 4 bits).
 * @param bg Background color (low 4 bits).
 */
void fb_write_cell(unsigned int i, char c, unsigned char fg, unsigned char bg)
{
    unsigned int offset = 2 * i;
    fb[offset] = c; //1° turn. In this part we define the character of the element that we are putting in the framebuffer.
    fb[offset + 1] = ((bg & 0x0F) << 4) | (fg & 0x0F); // 2° turn. In this section, we define the foreground color and the background color
}

/**
 * Scrolls the framebuffer up by one row and keeps the cursor on the last row.
 */
void scroll()
{
    unsigned int i;

    // Moves all rows up by 1 line
    for (i = 0; i < (FB_HEIGHT - 1) * FB_WIDTH; i++)
    {
        fb_write_cell(i, fb[2 * (i + FB_WIDTH)], FB_GREEN, FB_BLACK);
    }

    // Clears the last line on the frame buffer
    for (i = (FB_HEIGHT - 1) * FB_WIDTH; i < FB_HEIGHT * FB_WIDTH; i++)
    {
        fb_write_cell(i, ' ', FB_GREEN, FB_BLACK);
    }

    cursor_pos = (FB_HEIGHT - 1) * FB_WIDTH;
}

/**
 * Clears the full text framebuffer and places the cursor at the top-left cell.
 */
void fb_clear_screen()
{
    unsigned int i;

    for (i = 0; i < FB_WIDTH * FB_HEIGHT; i++)
    {
        fb_write_cell(i, ' ', FB_GREEN, FB_BLACK);
    }

    cursor_pos = 0;
    fb_move_cursor(cursor_pos);
}

/**
 * Counts the number of visible characters in a null-terminated string.
 *
 * @param buf Pointer to a null-terminated string.
 * @return Length of the string, excluding the null terminator.
 */
static unsigned int fb_strlen(const char *buf)
{
    unsigned int len = 0U;

    if (buf == 0)
    {
        return 0U;
    }

    while (buf[len] != '\0')
    {
        len++;
    }

    return len;
}

/**
 * Writes a null-terminated string to the framebuffer using the current cursor position.
 * Newline characters move the cursor to the next row and scrolling is applied as needed.
 *
 * @param buf Pointer to the text that will be printed.
 * @return Number of characters processed from buf.
 */
int fb_write(const char *buf)
{
    unsigned int i;
    unsigned int len = fb_strlen(buf);

    for (i = 0; i < len; i++)
    {
        char c = buf[i];

        if (c == '\n')
        {
            cursor_pos += FB_WIDTH - (cursor_pos % FB_WIDTH);
        }
        else
        {
            fb_write_cell(cursor_pos, c, FB_GREEN, FB_BLACK);
            cursor_pos++;
        }

        if (cursor_pos >= FB_WIDTH * FB_HEIGHT)
        {
            scroll(); // If it's full, then scrolls the framebuffer
        }
    }

    fb_move_cursor(cursor_pos); // Updates the frame buffer cursor

    return (int)len;
}

/**
 * Moves the cursor one cell to the left when possible.
 */
void fb_decrement_cursor_pos()
{
    if (cursor_pos == 0)
    {
        return; // Invalid operation, because the cursor is already at 0
    }

    cursor_pos--;
    fb_move_cursor(cursor_pos);
}