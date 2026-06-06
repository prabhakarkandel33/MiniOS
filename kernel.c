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
#include "tss.h"
#include "shell.h"
#include "process.h"
#include "syscall.h"
#include "ramfs.h"

#if defined(__linux__)
#error "You are not using a cross compiler, expect trouble"
#endif

volatile uint32_t tick = 0;
static volatile uint8_t multitasking_ready = 0;

extern uint8_t _binary_hello_user_elf_start[];
extern uint8_t _binary_hello_user_elf_end[];

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

    uint32_t esp;
    __asm__ volatile ("mov %%esp, %0" : "=r"(esp));
    tss_install(esp);
    tss_flush();

    ramfs_init();
    ramfs_create_ref("hello.elf",
        _binary_hello_user_elf_start,
        (uint32_t)(_binary_hello_user_elf_end - _binary_hello_user_elf_start));
    ramfs_create("readme.txt", (uint8_t*)"Welcome to prabhakarOS!\n", 24);
    ramfs_create("hello.txt",  (uint8_t*)"Hello, World!\n", 14);

    terminal_setcolor(vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("Kernel booted.\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));

    create_kernel_task(shell_run, "shell");

    multitasking_ready = 1;
    for (;;) __asm__ volatile ("hlt");
}