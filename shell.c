#include "shell.h"
#include "terminal.h"
#include "keyboard.h"
#include "pmm.h"
#include "task.h"
#include "heap.h"
#include <stdint.h>
#include <stddef.h>
#include "ramfs.h"
#include "process.h"

#define INPUT_BUFFER_SIZE 256

static char input_buffer[INPUT_BUFFER_SIZE];
static int  input_len = 0;

// -------------------------------------------------------
// helper: strcmp
// -------------------------------------------------------
static int shell_strcmp(const char* a, const char* b) {
    while (*a && *b && *a == *b) { a++; b++; }
    return *a - *b;
}

// -------------------------------------------------------
// helper: strlen
// -------------------------------------------------------
static int shell_strlen(const char* s) {
    int n = 0;
    while (s[n]) n++;
    return n;
}

// -------------------------------------------------------
// print the prompt
// -------------------------------------------------------
static void print_prompt(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("root");
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    terminal_writestring("@");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("prabhakarOS");
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    terminal_writestring("-> ");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
}

// -------------------------------------------------------
// built-in: help
// -------------------------------------------------------
static void cmd_help(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("Available commands:\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    terminal_writestring("  help     - show this message\n");
    terminal_writestring("  clear    - clear the screen\n");
    terminal_writestring("  echo     - print text\n");
    terminal_writestring("  meminfo  - show memory stats\n");
    terminal_writestring("  tasks    - show task info\n");
    terminal_writestring("  version  - show kernel version\n");
    terminal_writestring("  ls       - list files\n");
    terminal_writestring("  cat      - print file contents\n");
    terminal_writestring("  rm       - delete a file\n");
    terminal_writestring("  write    - write content to file\n");
    terminal_writestring("  exec     - execute ELF file\n");
}

// -------------------------------------------------------
// built-in: clear
// -------------------------------------------------------
static void cmd_clear(void) {
    terminal_initialize();
}

// -------------------------------------------------------
// built-in: echo
// -------------------------------------------------------
static void cmd_echo(const char* args) {
    terminal_writestring(args);
    terminal_writestring("\n");
}

// -------------------------------------------------------
// built-in: meminfo
// -------------------------------------------------------
static void cmd_meminfo(void) {
    pmm_print_stats();
}

// -------------------------------------------------------
// built-in: version
// -------------------------------------------------------
static void cmd_version(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("prabhakarOS v0.1\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    terminal_writestring("Built with i686-elf-gcc\n");
    terminal_writestring("Higher half kernel, paging, multitasking, user mode\n");
}

// -------------------------------------------------------
// built-in: tasks
// -------------------------------------------------------
static void cmd_tasks(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("current task: ");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    terminal_writestring(current_task->name);
    terminal_writestring("  id=");
    terminal_writehex(current_task->id);
    terminal_writestring("  time=");
    terminal_writehex(current_task->time_used);
    terminal_writestring("\n");
}

// -------------------------------------------------------
// process one command
// -------------------------------------------------------
static void process_command(void) {
    // skip leading spaces
    int i = 0;
    while (input_buffer[i] == ' ')
        i++;

    char* cmd = input_buffer + i;

    if (shell_strlen(cmd) == 0)
        return;

    // find where args start (after first word)
    int j = i;
    while (input_buffer[j] && input_buffer[j] != ' ')
        j++;

    char* args = input_buffer + j;

    if (*args == ' ')
        args++;  // skip space between cmd and args

    // null terminate the command word temporarily
    char saved = input_buffer[j];
    input_buffer[j] = '\0';

    if (shell_strcmp(cmd, "help") == 0) {
        cmd_help();

    } else if (shell_strcmp(cmd, "clear") == 0) {
        cmd_clear();

    } else if (shell_strcmp(cmd, "echo") == 0) {
        cmd_echo(args);

    } else if (shell_strcmp(cmd, "meminfo") == 0) {
        cmd_meminfo();

    } else if (shell_strcmp(cmd, "version") == 0) {
        cmd_version();

    } else if (shell_strcmp(cmd, "tasks") == 0) {
        cmd_tasks();

    } else if (shell_strcmp(cmd, "ls") == 0) {
        ramfs_list();

    } else if (shell_strcmp(cmd, "cat") == 0) {

        if (shell_strlen(args) == 0) {
            terminal_writestring("usage: cat <filename>\n");
        } else {
            ramfs_node_t* node = ramfs_find(args);

            if (!node) {
                terminal_writestring("cat: file not found: ");
                terminal_writestring(args);
                terminal_writestring("\n");

            } else if (!node->data) {
                terminal_writestring("(empty file)\n");

            } else {
                for (uint32_t k = 0; k < node->size; k++)
                    terminal_putchar((char)node->data[k]);

                terminal_putchar('\n');
            }
        }

    } else if (shell_strcmp(cmd, "rm") == 0) {

        if (shell_strlen(args) == 0) {
            terminal_writestring("usage: rm <filename>\n");
        } else {
            ramfs_delete(args);
            terminal_writestring("deleted: ");
            terminal_writestring(args);
            terminal_writestring("\n");
        }

    } else if (shell_strcmp(cmd, "exec") == 0) {

        if (shell_strlen(args) == 0) {
            terminal_writestring("usage: exec <filename>\n");
        } else {
            process_t* proc = process_create_from_elf(args);
            if (!proc) {
                terminal_writestring("exec failed\n");
            }
        }

    } else if (shell_strcmp(cmd, "write") == 0) {

        // usage: write filename content
        if (shell_strlen(args) == 0) {
            terminal_writestring("usage: write <filename> <content>\n");

        } else {

            // split args into filename and content
            int k = 0;
            while (args[k] && args[k] != ' ')
                k++;

            if (args[k] == '\0') {
                terminal_writestring("usage: write <filename> <content>\n");

            } else {

                char filename[64];
                int m = 0;

                while (m < k && m < 63) {
                    filename[m] = args[m];
                    m++;
                }

                filename[m] = '\0';

                char* content = args + k + 1;
                uint32_t len = (uint32_t)shell_strlen(content);

                if (ramfs_write(filename, (uint8_t*)content, len) == 0) {
                    terminal_writestring("written: ");
                    terminal_writestring(filename);
                    terminal_writestring("\n");

                } else {
                    terminal_writestring("write failed\n");
                }
            }
        }

    } else {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
        terminal_writestring("Unknown command: ");

        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
        terminal_writestring(cmd);
        terminal_writestring("\n");
        terminal_writestring("Type 'help' for available commands.\n");
    }

    // restore original buffer
    input_buffer[j] = saved;
}

// -------------------------------------------------------
// handle one keypress — called from keyboard_handler
// -------------------------------------------------------
void shell_handle_key(char c) {
    if (c == '\n') {
        terminal_putchar('\n');
        input_buffer[input_len] = '\0';
        process_command();
        input_len = 0;
        print_prompt();
    }
    else if (c == '\b') {
        if (input_len > 0) {
            input_len--;
            terminal_putchar('\b');
        }
    }
    else if (input_len < INPUT_BUFFER_SIZE - 1) {
        input_buffer[input_len++] = c;
        terminal_putchar(c);
    }
}

// -------------------------------------------------------
// shell entry point — runs as a kernel task
// -------------------------------------------------------
void shell_run(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("\nWelcome to prabhakarOS!\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    terminal_writestring("Type 'help' for available commands.\n\n");
    print_prompt();

    // shell just waits — input comes via shell_handle_key
    for (;;) {
        __asm__ volatile ("hlt");
    }
}