// kernel.c
#include <stdint.h>
#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "terminal.h"
#include "paging.h"
#include "pmm.h"
#include "heap.h"
#include "task.h"
#include "semaphore.h"


semaphore_t* term_mutex;

#if defined(__linux__)
#error "You are not using a cross compiler, expect trouble"
#endif

volatile uint32_t tick = 0;
static volatile uint8_t multitasking_ready = 0;


void task_a(void);
void task_b(void);
void task_c(void);
void task_monitor(void);
void task_terminating(void);


void irq_handler(void) {
    tick++;
    pic_eoi(0);

    if (!multitasking_ready) return;

    if (time_slice_remaining > 0) {
        time_slice_remaining--;
    } else {
        schedule();    // slice expired or was 0 — switch now
    }
}


void kernel_main(void) {
    gdt_init();
    paging_init();
    terminal_initialize();
    pic_init();
    idt_init();
    pmm_init(32 * 1024 * 1024);
    heap_init();
    multitasking_init();

    terminal_setcolor(vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("Kernel booted.\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));

    term_mutex = mutex_create();
    create_kernel_task(task_a, "task_a");
    create_kernel_task(task_b, "task_b");
    multitasking_ready = 1;
    for (;;) __asm__ volatile ("hlt");
}

static volatile uint32_t a_count = 0;
static volatile uint32_t b_count = 0;


void task_a(void) {
    for (;;) {
        mutex_acquire(term_mutex);
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK));
        terminal_putchar('A');
        mutex_release(term_mutex);
        volatile uint32_t i = 0; while (i < 500000) i++;
    }
}

void task_b(void) {
    for (;;) {
        mutex_acquire(term_mutex);
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
        terminal_putchar('B');
        mutex_release(term_mutex);
        volatile uint32_t i = 0; while (i < 500000) i++;
    }
}

void task_monitor(void) {
    for (;;) {
        volatile uint32_t i = 0; while (i < 5000000) i++;
        // terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        // terminal_writestring("\nA=");
        // terminal_writehex(a_count);
        // terminal_writestring(" B=");
        // terminal_writehex(b_count);
        // terminal_writestring(" tick=");
        // terminal_writehex(tick);
        // terminal_writestring("\n");
    }
}

void task_terminating(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("task starting\n");
    volatile uint32_t i = 0; while (i < 3000000) i++;
    terminal_writestring("task terminating\n");
    terminate_task();
    // should never reach here
    terminal_writestring("ERROR: still running\n");
}