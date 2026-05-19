// pic.h
#ifndef PIC_H
#define PIC_H

#include <stdint.h>
#include "io.h"          // inb/outb/io_wait live here now

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

#define PIC1_OFFSET  0x20
#define PIC2_OFFSET  0x28

#define PIC_EOI      0x20

void pic_init(void);
void pic_eoi(uint8_t irq);

#endif