// terminal.c
#include "terminal.h"
#include <stddef.h>
#include <stdint.h>
#include "io.h"

#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

static size_t    terminal_row;
static size_t    terminal_column;
static uint8_t   terminal_color;
static uint16_t* terminal_buffer = (uint16_t*)0xC03FF000;
static size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

static void update_cursor(void) {
    uint16_t pos = terminal_row * VGA_WIDTH + terminal_column;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t)uc | (uint16_t)color << 8;
}

uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

uint8_t terminal_getcolor(void) {
    return terminal_color;
}

void terminal_initialize(void) {
    terminal_row    = 0;
    terminal_column = 0;
    terminal_color  = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    for (size_t y = 0; y < VGA_HEIGHT; y++)
        for (size_t x = 0; x < VGA_WIDTH; x++)
            terminal_buffer[y * VGA_WIDTH + x] = vga_entry(' ', terminal_color);
    
    update_cursor();
}

void terminal_setcolor(uint8_t color) {
    terminal_color = color;
}

static void terminal_scroll(void) {
    for (size_t y = 0; y < VGA_HEIGHT - 1; y++)
        for (size_t x = 0; x < VGA_WIDTH; x++)
            terminal_buffer[y * VGA_WIDTH + x] = terminal_buffer[(y+1) * VGA_WIDTH + x];
    for (size_t x = 0; x < VGA_WIDTH; x++)
        terminal_buffer[(VGA_HEIGHT-1) * VGA_WIDTH + x] = vga_entry(' ', terminal_color);
}

void terminal_putchar(char c) {
    __asm__ volatile ("cli");

    if (terminal_row >= VGA_HEIGHT) terminal_row = VGA_HEIGHT - 1;
    if (terminal_column >= VGA_WIDTH) terminal_column = VGA_WIDTH - 1;

    if (c == '\n') {
        terminal_column = 0;
        if (++terminal_row >= VGA_HEIGHT) {
            terminal_scroll();
            terminal_row = VGA_HEIGHT - 1;
        }
        __asm__ volatile ("sti");
        return;
    }

    // handle backspace
    if (c == '\b') {
        if (terminal_column > 0) {
            terminal_column--;
        } else if (terminal_row > 0) {
            // wrap to end of previous line
            terminal_row--;
            terminal_column = VGA_WIDTH - 1;
        }
        // write space to erase the character visually
        terminal_buffer[terminal_row * VGA_WIDTH + terminal_column] =
            vga_entry(' ', terminal_color);
        __asm__ volatile ("sti");
        return;
    }

    terminal_buffer[terminal_row * VGA_WIDTH + terminal_column] =
        vga_entry(c, terminal_color);

    if (++terminal_column >= VGA_WIDTH) {
        terminal_column = 0;
        if (++terminal_row >= VGA_HEIGHT) {
            terminal_scroll();
            terminal_row = VGA_HEIGHT - 1;
        }
    }
    update_cursor();

    __asm__ volatile ("sti");
}

void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++)
        terminal_putchar(data[i]);
}

void terminal_writestring(const char* data) {
    terminal_write(data, strlen(data));
}

// prints a 32-bit value as 8 hex digits — useful for addresses and error codes
void terminal_writehex(uint32_t value) {
    const char* hex = "0123456789ABCDEF";
    char buf[11];        // "0x" + 8 digits + null
    buf[0]  = '0';
    buf[1]  = 'x';
    for (int i = 9; i >= 2; i--) {
        buf[i] = hex[value & 0xF];
        value >>= 4;
    }
    buf[10] = '\0';
    terminal_writestring(buf);
}