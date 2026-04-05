#ifndef INCLUDE_SHELL_H
#define INCLUDE_SHELL_H

/*
 * Initializes shell state and prints the initial prompt.
 */
void shell_init(void);

/*
 * Processes one character received from keyboard input.
 * key is the translated ASCII character (or control character) to handle.
 */
void shell_handle_key(char key);

#endif
