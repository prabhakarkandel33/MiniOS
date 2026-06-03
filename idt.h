#ifndef IDT_H
#define IDT_H

#include <stdint.h>

//one idt entry

typedef struct{
    uint16_t isr_low; //bits 0-15 of handler
    uint16_t kernel_cs; //which gdt segment to use
    uint8_t reserved;//zero
    uint8_t attributes; //game type + priviledge level + present bit
    uint16_t isr_high; //higher bits of handler 16-31
} __attribute__((packed)) idt_entry_t;


//what you pass to ldt instruction
typedef struct{
    uint16_t limit;
    uint32_t base; //address of idt array
} __attribute__((packed)) idtr_t;


void idt_init(void);

#endif