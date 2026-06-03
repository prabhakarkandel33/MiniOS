// idt.c
#include "idt.h"
#include "pic.h"
#include "terminal.h"
#include <stdint.h>

#define IDT_MAX_DESCRIPTORS 256

__attribute__((aligned(0x10)))
static idt_entry_t idt[IDT_MAX_DESCRIPTORS];

static idtr_t idtr;

extern void* isr_stub_table[];
extern void* irq_stub_table[];

static const char* exception_names[] = {
    "Division by zero",       // 0
    "Debug",                  // 1
    "Non-maskable interrupt", // 2
    "Breakpoint",             // 3
    "Overflow",               // 4
    "Bound range exceeded",   // 5
    "Invalid opcode",         // 6
    "Device not available",   // 7
    "Double fault",           // 8
    "Coprocessor overrun",    // 9
    "Invalid TSS",            // 10
    "Segment not present",    // 11
    "Stack segment fault",    // 12
    "General protection",     // 13
    "Page fault",             // 14
    "Reserved",               // 15
    "x87 FPU error",          // 16
    "Alignment check",        // 17
    "Machine check",          // 18
    "SIMD FPU exception",     // 19
    "Virtualization",         // 20
    "Reserved",               // 21
    "Reserved",               // 22
    "Reserved",               // 23
    "Reserved",               // 24
    "Reserved",               // 25
    "Reserved",               // 26
    "Reserved",               // 27
    "Reserved",               // 28
    "Reserved",               // 29
    "Security exception",     // 30
    "Reserved",               // 31
};

static void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags) {
    idt_entry_t* entry     = &idt[vector];
    entry->isr_low         = (uint32_t)isr & 0xFFFF;
    entry->kernel_cs       = 0x08;
    entry->reserved        = 0;
    entry->attributes      = flags;
    entry->isr_high        = (uint32_t)isr >> 16;
}

// called from isr.s with vector number as argument
__attribute__((noreturn))
void exception_handler(uint32_t vector) {
    uint32_t cr2;
    __asm__ volatile ("mov %%cr2, %0" : "=r"(cr2));
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_RED));
    terminal_writestring("\n*** EXCEPTION ");
    terminal_writehex(vector);
    terminal_writestring(": ");
    if (vector < 32)
        terminal_writestring(exception_names[vector]);
    terminal_writestring(" ***\n");
    terminal_writestring("CR2=");
    terminal_writehex(cr2);
    terminal_writestring("\n");
    terminal_writestring("System halted.\n");
    __asm__ volatile ("cli; hlt");
    __builtin_unreachable();
}

void idt_init(void) {
    idtr.base  = (uint32_t)&idt[0];
    idtr.limit = (uint16_t)(sizeof(idt_entry_t) * IDT_MAX_DESCRIPTORS) - 1;

    for (uint8_t vector = 0; vector < 32; vector++)
        idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);

    for (uint8_t irq = 0; irq < 16; irq++)
        idt_set_descriptor(32 + irq, irq_stub_table[irq], 0x8E);

    __asm__ volatile ("lidt %0" : : "m"(idtr));
    __asm__ volatile ("sti");
}