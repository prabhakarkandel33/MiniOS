#include "pic.h"

#define ICW1_INIT 0x10
#define ICW1_ICW4 0x01
#define ICW4_8086 0x01

void pic_init(void){
    //save current interrupt mask to restore em
    //masks control which irqs are allowed
    // uint8_t mask1 = inb(PIC1_DATA);
    // uint8_t mask2 = inb(PIC2_DATA);

    //initialize- puts pic to expect mode
    outb(PIC1_COMMAND,ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();

    //set vector offsets
    outb(PIC1_DATA,PIC1_OFFSET);
    io_wait();
    outb(PIC2_DATA,PIC2_OFFSET);
    io_wait();

    //tell master there is slave on irq2
    outb(PIC1_DATA,0x04);
    io_wait();
    outb(PIC2_DATA,0x02);
    io_wait();

    //set 8086 mode
    outb(PIC1_DATA,ICW4_8086);
    io_wait();
    outb(PIC2_DATA,ICW4_8086);
    io_wait();

    //restore masks
    outb(PIC1_DATA, 0x00);
    outb(PIC2_DATA, 0x00);

}

void pic_eoi(uint8_t irq){
    //if irq came from slave notify both PICs
    if (irq >= 8){
        outb(PIC2_COMMAND,PIC_EOI);
    }
    //always notify master
    outb(PIC1_COMMAND,PIC_EOI);
}


