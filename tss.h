#ifndef TSS_H
#define TSS_H

#include <stdint.h>

typedef struct {
    uint16_t link,    link_h;
    uint32_t esp0;          // kernel stack pointer — CPU loads this on interrupt
    uint16_t ss0,     ss0_h; // kernel stack segment
    uint32_t esp1;
    uint16_t ss1,     ss1_h;
    uint32_t esp2;
    uint16_t ss2,     ss2_h;
    uint32_t cr3, eip, eflags;
    uint32_t eax, ecx, edx, ebx;
    uint32_t esp, ebp, esi, edi;
    uint16_t es,  es_h;
    uint16_t cs,  cs_h;
    uint16_t ss,  ss_h;
    uint16_t ds,  ds_h;
    uint16_t fs,  fs_h;
    uint16_t gs,  gs_h;
    uint16_t ldt, ldt_h;
    uint16_t trap, iomap;
} __attribute__((packed)) tss_t;

extern tss_t sys_tss;

void tss_install(uint32_t kernel_stack);
void tss_flush(void);
void tss_set_kernel_stack(uint32_t stack);

#endif