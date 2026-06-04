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
#include "process.h"
#include "syscall.h"
#include "ramfs.h"


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

extern void switch_to_user_mode(uint32_t user_stack);

static uint32_t setup_user_stack(void) {
    // allocate one physical frame for user stack
    pageframe_t frame = pmm_alloc_frame();
    if (frame == PMM_ERROR) {
        terminal_writestring("ERROR: no frame for user stack\n");
        return 0;
    }

    // map it at USER_STACK_VIRT with user accessible flag
    paging_map(USER_STACK_VIRT, frame);

    // stack grows downward — return top of stack
    return USER_STACK_VIRT + USER_STACK_SIZE;
}

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

void user_process_entry(void) {
    // this runs as a kernel task with its own page directory
    // eventually this will switch_to_user_mode
    terminal_writestring("user process running!\n");
    for (;;) __asm__ volatile ("hlt");
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

    uint32_t esp;
    __asm__ volatile ("mov %%esp, %0" : "=r"(esp));
    tss_install(esp);
    tss_flush();

    ramfs_init();
    ramfs_create("readme.txt", (uint8_t*)"Welcome to prabhakarOS!\n", 24);
    ramfs_create("hello.txt", (uint8_t*)"Hello, World!\n", 14);

    // debug — list right after creating
    terminal_writestring("files after create:\n");
    ramfs_list();
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("Kernel booted.\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));

    process_create(0, "init");
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



