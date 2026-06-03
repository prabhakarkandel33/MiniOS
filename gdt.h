#ifndef GDT_H
#define GDT_H

#include <stdint.h>

//One gdt entry

struct gdt_entry {
    uint16_t limit_low;     // lower 16 bits of limit
    uint16_t base_low;      // lower 16 bits of base
    uint8_t  base_middle;   // next 8 bits of base
    uint8_t  access;        // access flags (ring level, type, etc.)
    uint8_t  granularity;   // upper 4 bits = flags, lower 4 = limit[19:16]
    uint8_t  base_high;     // last 8 bits of base
} __attribute__((packed));

// GDTR — what you pass to LGDT instruction
struct gdt_ptr {
    uint16_t limit;         // size of GDT - 1
    uint32_t base;          // address of GDT
} __attribute__((packed));

void gdt_init(void);

#endif