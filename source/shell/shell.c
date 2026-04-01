#include "shell/shell.h"
#include "io/framebuffer.h"

#define SHELL_INPUT_MAX 128U
#define SHELL_PROMPT "> "

static char shell_line_buffer[SHELL_INPUT_MAX];
static unsigned int shell_line_len = 0U;
static unsigned char shell_ready = 0U;

/* Returns non-zero when c is a printable ASCII character. */
static int shell_is_printable(char c)
{
    return (c >= 32 && c <= 126);
}

/* Advances text past spaces and tabs and returns the first non-blank character. */
static const char *shell_skip_spaces(const char *text)
{
    while (*text == ' ' || *text == '\t')
    {
        text++;
    }

    return text;
}

/*
 * Splits one input line into command and argument pointers.
 * line is the full null-terminated user input.
 * command receives the command token.
 * command_capacity is the size of command in bytes.
 * args receives a pointer to the first argument character in line.
 */
static void shell_parse_command(const char *line, char *command, unsigned int command_capacity, const char **args)
{
    const char *cursor = shell_skip_spaces(line);
    unsigned int i = 0U;

    while (cursor[i] != '\0' && cursor[i] != ' ' && cursor[i] != '\t' && (i + 1U) < command_capacity)
    {
        command[i] = cursor[i];
        i++;
    }
    command[i] = '\0';

    cursor += i;
    *args = shell_skip_spaces(cursor);
}

/* Writes the shell prompt to the framebuffer. */
static void shell_print_prompt(void)
{
    fb_write(SHELL_PROMPT);
}

/* Clears the current input line state in the shell buffer. */
static void shell_reset_line(void)
{
    shell_line_len = 0U;
    shell_line_buffer[0] = '\0';
}

/* Deletes one character from the current line and mirrors it on screen. */
static void shell_erase_last_char(void)
{
    if (shell_line_len == 0U)
    {
        return;
    }

    shell_line_len--;
    shell_line_buffer[shell_line_len] = '\0';

    fb_decrement_cursor_pos();
    fb_write(" ");
    fb_decrement_cursor_pos();
}

/*
 * Executes one parsed input line.
 * line is the full command line text entered by the user.
 */
static void shell_execute_line(const char *line)
{
    char command[32];
    const char *args;

    shell_parse_command(line, command, sizeof(command), &args);

    if (command[0] == '\0')
    {
        return;
    }

    fb_write("Comando nao encontrado: ");
    fb_write(command);
    fb_write("\n");

    (void)args;
}

/* Initializes shell runtime state and prints the first prompt. */
void shell_init(void)
{
    shell_reset_line();
    shell_ready = 1U;
    shell_print_prompt();
}

/*
 * Handles one keyboard character and updates shell state/output.
 * key is the translated character from the keyboard driver.
 */
void shell_handle_key(char key)
{
    char out[2];

    if (shell_ready == 0U || key == '\0')
    {
        return;
    }

    if (key == '\b')
    {
        shell_erase_last_char();
        return;
    }

    if (key == '\n')
    {
        fb_write("\n");
        shell_line_buffer[shell_line_len] = '\0';
        shell_execute_line(shell_line_buffer);
        shell_reset_line();
        shell_print_prompt();
        return;
    }

    if (!shell_is_printable(key))
    {
        return;
    }

    if (shell_line_len >= (SHELL_INPUT_MAX - 1U))
    {
        return;
    }

    shell_line_buffer[shell_line_len] = key;
    shell_line_len++;
    shell_line_buffer[shell_line_len] = '\0';

    out[0] = key;
    out[1] = '\0';
    fb_write(out);
}
