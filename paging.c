// paging.c
#include "paging.h"
#include "pmm.h"
#include "terminal.h"
#include <stdint.h>

#define KERNEL_VIRT_BASE 0xC0000000


static uint32_t page_directory[PAGE_DIR_SIZE] __attribute__((aligned(4096)));
static uint32_t first_table[1024]             __attribute__((aligned(4096)));

// small pool of page tables for directories 1-15 (covers first 64MB)
// lives in kernel BSS — always mapped — no chicken-and-egg
#define PTABLE_POOL_SIZE 16
static uint32_t ptable_pool[PTABLE_POOL_SIZE][1024] __attribute__((aligned(4096)));
static uint8_t  ptable_used[PTABLE_POOL_SIZE] = {0};

extern void load_page_directory(uint32_t* dir);
extern void enable_paging(void);

static uint32_t* alloc_page_table(void) {
    for (int i = 0; i < PTABLE_POOL_SIZE; i++) {
        if (!ptable_used[i]) {
            ptable_used[i] = 1;
            // zero it
            for (int j = 0; j < 1024; j++)
                ptable_pool[i][j] = PAGE_WRITABLE;
            return ptable_pool[i];
        }
    }
    terminal_writestring("PAGING: page table pool exhausted!\n");
    return 0;
}

void paging_map(uint32_t virt, uint32_t phys) {
    uint32_t dir_index   = virt >> 22;
    uint32_t table_index = (virt >> 12) & 0x3FF;

    if (!(page_directory[dir_index] & PAGE_PRESENT)) {
        uint32_t* new_table = alloc_page_table();
        if (!new_table) return;
        page_directory[dir_index] = (uint32_t)new_table | PAGE_PRESENT | PAGE_WRITABLE;
    }

    uint32_t* table = (uint32_t*)(page_directory[dir_index] & ~0xFFF);
    table[table_index] = phys | PAGE_PRESENT | PAGE_WRITABLE;

    __asm__ volatile ("invlpg (%0)" : : "r"(virt) : "memory");
}

void paging_init(void) {
    

    for (int i = 0; i < PAGE_DIR_SIZE; i++)
        page_directory[i] = PAGE_WRITABLE;

    // identity map first 4MB (physical) — needed during transition
    for (int i = 0; i < PAGE_TABLE_SIZE; i++)
        first_table[i] = (i * PAGE_SIZE) | PAGE_PRESENT | PAGE_WRITABLE;

    // map first 4MB at both 0x00000000 and 0xC0000000
    page_directory[0]   = (uint32_t)first_table | PAGE_PRESENT | PAGE_WRITABLE;
    page_directory[768] = (uint32_t)first_table | PAGE_PRESENT | PAGE_WRITABLE;

    load_page_directory(page_directory);
    enable_paging();
}