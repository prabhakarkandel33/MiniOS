#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include "pmm.h"

#define PAGE_DIR_SIZE   1024
#define PAGE_TABLE_SIZE 1024
#define PAGE_SIZE       4096

#define PAGE_PRESENT    0x1
#define PAGE_WRITABLE   0x2
#define PAGE_USER       0x4

void paging_init(void);
void paging_map(uint32_t virt, uint32_t phys);

#endif