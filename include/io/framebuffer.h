#ifndef INCLUDE_FRAMEBUFFER_H
#define INCLUDE_FRAMEBUFFER_H

void fb_move_cursor(unsigned short pos);


void fb_write_cell(unsigned int i, char c, unsigned char fg, unsigned char bg);


void scroll();

/* Clears the entire text screen and resets the cursor to the first cell. */
void fb_clear_screen();

int fb_write(const char *buf);

void fb_decrement_cursor_pos();

#endif