#include "shell.h"
#include "terminal.h"
#include "keyboard.h"
#include "pmm.h"
#include "task.h"
#include "heap.h"
#include <stdint.h>
#include <stddef.h>

#define INPUT_BUFFER_SIZE 256
#define MAX_ARGS          16

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
// helper: strncmp
// -------------------------------------------------------
static int shell_strncmp(const char* a, const char* b, int n) {
    while (n-- && *a && *b && *a == *b) { a++; b++; }
    if (n < 0) return 0;
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
    while (input_buffer[i] == ' ') i++;

    char* cmd = input_buffer + i;
    if (shell_strlen(cmd) == 0) return;

    // find where args start (after first word)
    int j = i;
    while (input_buffer[j] && input_buffer[j] != ' ') j++;
    char* args = input_buffer + j;
    if (*args == ' ') args++;  // skip space between cmd and args

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
    } else {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
        terminal_writestring("Unknown command: ");
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
        terminal_writestring(cmd);
        terminal_writestring("\n");
        terminal_writestring("Type 'help' for available commands.\n");
    }

    input_buffer[j] = saved;  // restore
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
            terminal_putchar('\b');  // one call handles everything now
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
    // called from keyboard_handler
    for (;;) {
        __asm__ volatile ("hlt");
    }
}