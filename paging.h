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


#define USER_CODE_VIRT   0x00100000   // user code loads here
#define USER_STACK_VIRT  0x00800000   // user stack here
#define USER_STACK_SIZE  4096


void paging_init(void);
void paging_map(uint32_t virt, uint32_t phys);
void paging_debug(void);
extern uint32_t page_directory[];



uint32_t* paging_create_user_directory(void);


void paging_map_in(uint32_t* dir, uint32_t virt, uint32_t phys);

#endif