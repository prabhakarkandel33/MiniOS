// kernel.c
#include <stdint.h>
#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "terminal.h"
#include "paging.h"
#include "pmm.h"
#include "heap.h"

#if defined(__linux__)
#error "You are not using a cross compiler, expect trouble"
#endif

static uint32_t tick = 0;

void irq_handler(void) {
    tick++;
    pic_eoi(0);
}

void kernel_main(void) {
    gdt_init();
    paging_init();
    terminal_initialize();
    pic_init();
    idt_init();
    pmm_init(32 * 1024 * 1024);
    heap_init();

    terminal_writestring("kernel start=");

    

    for (;;) __asm__ volatile ("hlt");
}