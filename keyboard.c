// keyboard.c
#include "keyboard.h"
#include "pic.h"
#include "terminal.h"    // replaces extern void terminal_putchar()

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
        if (c) terminal_putchar(c);
    }
    pic_eoi(1);
}