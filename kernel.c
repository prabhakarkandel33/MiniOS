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
#include "tss.h"
#include "shell.h"
#include "elf.h"

semaphore_t* term_mutex;

#if defined(__linux__)
#error "You are not using a cross compiler, expect trouble"
#endif

volatile uint32_t tick = 0;
static volatile uint8_t multitasking_ready = 0;

extern uint8_t _binary_test_module_o_start[];
extern uint8_t _binary_test_module_o_end[];
extern uint32_t _binary_test_module_o_size;


void task_a(void);
void task_b(void);
void task_c(void);
void task_monitor(void);
void task_terminating(void);

extern void switch_to_user_mode(void);

void irq_handler(void) {
    tick++;
    pic_eoi(0);

    if (!multitasking_ready) return;

    if (time_slice_remaining > 0) {
        time_slice_remaining--;
    } else {
        schedule();
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

    // set up TSS
    uint32_t esp;
    __asm__ volatile ("mov %%esp, %0" : "=r"(esp));
    tss_install(esp);
    tss_flush();

    terminal_setcolor(vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("Kernel booted.\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));

   
    create_kernel_task(shell_run, "shell");

    multitasking_ready = 1;
    for (;;) __asm__ volatile ("hlt");
}

void task_a(void) {
    for (;;) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK));
        terminal_putchar('A');
        volatile uint32_t i = 0; while (i < 500000) i++;
    }
}

void task_b(void) {
    for (;;) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
        terminal_putchar('B');
        volatile uint32_t i = 0; while (i < 500000) i++;
    }
}

void task_monitor(void) {
    for (;;) {
        volatile uint32_t i = 0; while (i < 5000000) i++;
    }
}

void task_terminating(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("task starting\n");
    volatile uint32_t i = 0; while (i < 3000000) i++;
    terminal_writestring("task terminating\n");
    terminate_task();
    terminal_writestring("ERROR: still running\n");
}