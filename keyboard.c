#include "keyboard.h"
#include "pic.h"
#include "terminal.h"
#include "shell.h"
#include "syscall.h"
#include "process.h"


static const char scancode_table[128] = {
    0,    0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-',  '=', '\b', '\t',
   'q',  'w',  'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[',  ']', '\n',  0,  'a',  's',
   'd',  'f',  'g', 'h', 'j', 'k', 'l', ';','\'', '`',  0,  '\\','z',  'x', 'c',  'v',
   'b',  'n',  'm', ',', '.', '/',  0,   '*',  0,  ' ',  0,    0,   0,    0,   0,    0,
    0,    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,    0,   0,    0,   0,    0,
    0,    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,    0,   0,    0,   0,    0,
    0,    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,    0,   0,    0,   0,    0,
    0,    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,    0,   0,    0,   0,    0,
};

void keyboard_handler(void) {
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    if (!(scancode & 0x80)) {
        char c = scancode_table[scancode];
        if (c) {
            if (current_process) {
                // user process running — feed to syscall read buffer
                terminal_putchar(c);  // echo
                syscall_kbd_input(c);
            } else {
                shell_handle_key(c);
            }
        }
    }
    pic_eoi(1);
}