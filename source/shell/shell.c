#include "shell/shell.h"
#include "io/framebuffer.h"
#include "io/utils.h"
#include "file/vfs.h"

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
    // Temp path buffer
    char path_buffer[256]; 
    
    // Calls the vsf pwd function to fill the buffer with the path
    vfs_pwd(path_buffer, 256);
    
    fb_write(path_buffer);
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

static int shell_verify_command(char* command, char* args){
    if (command[0] == '\0')
    {
        return 0;
    } else if(!string_compare(command, "clear")) {
        fb_clear_screen();
        return 0;
    } else if(!string_compare(command, "pwd")){
        // Temp path buffer
        char path_buffer[256]; 
        
        // Calls the vsf pwd function to fill the buffer with the path
        vfs_pwd(path_buffer, 256);
        
        // Prints the pwd
        fb_write("PWD: ");
        fb_write(path_buffer);
        fb_write("\n");
        
        return 0;
    } else if(!string_compare(command, "touch")) {
        vfs_create(args, 0);

        return 0;
    } else if(!string_compare(command, "mkdir")) {
        vfs_create(args, 1);

        return 0;
    } else if(!string_compare(command, "cd")) {
        vfs_cd(args);

        return 0;
    } else if(!string_compare(command, "ls")) {
        vfs_ls(args);

        return 0;
    } else if(!string_compare(command, "rm")) {
        vfs_rm(args);

        return 0;
    }else if(!string_compare(command, "write")) {
        char filename[32];
        const char *content = "";
        unsigned int i = 0;

        // File name
        while (args[i] != '\0' && args[i] != ' ' && i < 31) {
            filename[i] = args[i];
            i++;
        }
        filename[i] = '\0';

        // Content start
        if (args[i] != '\0') {
            content = shell_skip_spaces(&args[i]);
        }

        // Verifies if the user sended the 2 arguments
        if (filename[0] == '\0' || content[0] == '\0') {
            fb_write("Uso: write <arquivo> <texto>\n");
            return 0;
        }

        // Calls the vfs function to write
        if (vfs_write(filename, content) != 0) {
            fb_write("Erro ao gravar arquivo.\n");
        };

        return 0;
    } else if(!string_compare(command, "read")) {
        // File name
        if (args[0] == '\0') {
            fb_write("Uso: read <arquivo>\n");
            return 0;
        }

        // Calls the vfs function to read
        char* file_content = vfs_read(args);
        
        if (file_content != NULL) {
            // Prints the content
            fb_write(file_content);
            fb_write("\n");
        } else {
            fb_write("Erro: Arquivo vazio ou nao encontrado.\n");
        }

        return 0;
    } else if(!string_compare(command, "stop")) {
        // Removes the virtual file system + the inodes from the memory
        fb_write("Limpando a memoria (VFS/RamFS)...\n");
        vfs_free_all();

        // Exit message
        fb_write("Sistema de arquivos desligado com seguranca.\n");
        fb_write("Pode fechar a janela do Bochs.\n");

        // Locks the processor in Kernel Mode.
        __asm__ __volatile__("cli"); // Clear interrupts
        while (1) {
            __asm__ __volatile__("hlt"); // Makes the processor sleep
        }
    }

    return 1;
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

    if(!shell_verify_command(command, args)) return;

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
