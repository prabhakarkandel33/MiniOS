#include "gdt.h"
#include "tss.h"

#define GDT_ENTRIES 6

static struct gdt_entry gdt[GDT_ENTRIES];
static struct gdt_ptr gp;

//declared in gdt_flush.s
extern void gdt_flush(uint32_t);

static void gdt_set_entry(int i, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran){
    gdt[i].base_low = (base & 0xFFFF);
    gdt[i].base_middle = (base >> 16) & 0xFF;
    gdt[i].base_high = (base >> 24) & 0xFF;

    gdt[i].limit_low = (limit & 0xFFFF);
    gdt[i].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt[i].access = access;    
}

void gdt_init(void) {
    gp.limit = (sizeof(struct gdt_entry) * GDT_ENTRIES) - 1;
    gp.base = (uint32_t)&gdt;   // virtual is fine — paging handles it
    gdt_set_entry(0, 0, 0,          0,    0);        // null
    gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);     // kernel code
    gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);     // kernel data
    gdt_set_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);     // user code
    gdt_set_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);     // user data

    // TSS entry — base = address of sys_tss, limit = size of TSS
    uint32_t tss_base = (uint32_t)&sys_tss - 0xC0000000;  // physical
    uint32_t tss_limit = sizeof(tss_t) - 1;
    gdt_set_entry(5, tss_base, tss_limit, 0x89, 0x00);

    gdt_flush((uint32_t)&gp);
}