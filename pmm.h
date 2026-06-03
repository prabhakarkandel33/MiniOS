#ifndef PMM_H
#define PMM_H

#include <stdint.h>

typedef uint32_t pageframe_t;

#define PMM_FREE 0x00
#define PMM_USED 0x01
#define PMM_ERROR 0xFFFFFFFF//when no frames available

//pass total ram in bytes
void pmm_init(uint32_t total_memory_bytes);

//allocate one 4kb physical frame returns physical address
pageframe_t pmm_alloc_frame(void);

//free a previously allocated frame
void pmm_free_frame(pageframe_t addr);

void pmm_print_stats(void);

uint32_t pmm_startframe(void);

#endif