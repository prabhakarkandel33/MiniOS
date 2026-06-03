#include "tss.h"
#include <stdint.h>

tss_t sys_tss;

void tss_install(uint32_t kernel_stack) {
    // zero the whole TSS
    uint8_t* p = (uint8_t*)&sys_tss;
    for (uint32_t i = 0; i < sizeof(tss_t); i++) p[i] = 0;

    sys_tss.ss0  = 0x10;           // kernel data segment
    sys_tss.esp0 = kernel_stack;   // kernel stack for Ring 0 returns
    sys_tss.iomap = sizeof(tss_t); // no IO port access from user mode
}

// called after switching tasks — update kernel stack in TSS
void tss_set_kernel_stack(uint32_t stack) {
    sys_tss.esp0 = stack;
}

void tss_flush(void) {
    // load TSS selector into TR register
    // 0x28 = GDT entry 5, RPL 0... wait we're adding as entry 5
    // selector = (5 << 3) | 0 = 0x28
    __asm__ volatile ("ltr %%ax" : : "a"(0x28));
}