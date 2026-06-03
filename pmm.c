#include "pmm.h"
#include "terminal.h"
#define KERNEL_VIRT_BASE 0xC0000000


// endkernel is a linker symbol — its ADDRESS is what matters, not its value
extern uint32_t endkernel;

// how many frames to grab at once internally
#define PREALLOC_COUNT 20

static uint8_t*   frame_map   = 0;   // bytemap: one byte per frame
static uint32_t   startframe  = 0;   // physical address of first managed frame
static uint32_t   npages      = 0;   // total number of managed frames

static pageframe_t pre_frames[PREALLOC_COUNT];  // batch of pre-grabbed frames

// -------------------------------------------------------
// internal: search bytemap for one free frame
// -------------------------------------------------------
static pageframe_t alloc_frame_internal(void) {
    for (uint32_t i = 0; i < npages; i++) {
        if (frame_map[i] == PMM_FREE) {
            frame_map[i] = PMM_USED;
            return startframe + (i * 0x1000);  // physical address
        }
    }
    return PMM_ERROR;   // out of memory
}

// -------------------------------------------------------
// public: initialize the allocator
// total_memory_bytes = how much RAM the machine has
// -------------------------------------------------------
void pmm_init(uint32_t total_memory_bytes) {
    frame_map = (uint8_t*)(&endkernel);  // virtual — OK for access
    npages    = total_memory_bytes / 0x1000;

    // compute startframe as PHYSICAL address
    uint32_t map_phys_end = ((uint32_t)(&endkernel) - KERNEL_VIRT_BASE) + npages;
    startframe = (map_phys_end + 0xFFF) & ~0xFFF;  // physical, 4KB aligned

    for (uint32_t i = 0; i < npages; i++)
        frame_map[i] = PMM_FREE;

    uint32_t used_frames = startframe / 0x1000;
    for (uint32_t i = 0; i < used_frames && i < npages; i++)
        frame_map[i] = PMM_USED;
}

// -------------------------------------------------------
// public: allocate one frame
// -------------------------------------------------------
static uint8_t  pre_index    = PREALLOC_COUNT;  // force refill on first call


pageframe_t pmm_alloc_frame(void) {
    if (pre_index >= PREALLOC_COUNT) {
        // batch exhausted — grab another 20
        for (int i = 0; i < PREALLOC_COUNT; i++) {
            pre_frames[i] = alloc_frame_internal();
        }
        pre_index = 0;
    }
    return pre_frames[pre_index++];
}

// -------------------------------------------------------
// public: free a frame
// -------------------------------------------------------
void pmm_free_frame(pageframe_t addr) {
    if (addr < startframe) return;   // can't free kernel memory

    uint32_t index = (addr - startframe) / 0x1000;
    if (index < npages) {
        frame_map[index] = PMM_FREE;
    }
}

// -------------------------------------------------------
// debug: print stats
// -------------------------------------------------------
void pmm_print_stats(void) {
    uint32_t used = 0;
    uint32_t free = 0;
    for (uint32_t i = 0; i < npages; i++) {
        if (frame_map[i] == PMM_USED) used++;
        else free++;
    }
    terminal_writestring("PMM: total=");
    terminal_writehex(npages * 0x1000);
    terminal_writestring(" used=");
    terminal_writehex(used * 0x1000);
    terminal_writestring(" free=");
    terminal_writehex(free * 0x1000);
    terminal_writestring("\n");
}

uint32_t pmm_startframe(void){
    return startframe;
}