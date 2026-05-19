#include "paging.h"
#include "heap.h"
#include "pmm.h"
#include "terminal.h"

#define HEAP_MAGIC    0xDEADBEEF   // marks a valid header
#define MIN_SPLIT     32           // don't split if remainder < this

typedef struct heap_block {
    uint32_t           magic;
    uint32_t           size;    // usable bytes (not counting header)
    uint8_t            free;
    struct heap_block* next;
} heap_block_t;

static heap_block_t* heap_start = 0;

// -------------------------------------------------------
// internal: ask PMM for n frames and return as one block
// -------------------------------------------------------
static heap_block_t* heap_grow(size_t size) {
    (void)size;

    pageframe_t base = pmm_alloc_frame();
    if (base == PMM_ERROR) return 0;

    paging_map(base, base);

    heap_block_t* block = (heap_block_t*)base;
    block->magic = HEAP_MAGIC;
    block->size  = PAGE_SIZE - sizeof(heap_block_t);
    block->free  = 1;
    block->next  = 0;
    return block;
}


// -------------------------------------------------------
// internal: split a block if it's much larger than needed
// -------------------------------------------------------
static void heap_split(heap_block_t* block, size_t size) {
    // only split if leftover is big enough to be useful
    if (block->size < size + sizeof(heap_block_t) + MIN_SPLIT)
        return;

    // carve a new block header right after the used portion
    heap_block_t* new_block = (heap_block_t*)((uint8_t*)block
                               + sizeof(heap_block_t) + size);
    new_block->magic = HEAP_MAGIC;
    new_block->size  = block->size - size - sizeof(heap_block_t);
    new_block->free  = 1;
    new_block->next  = block->next;

    block->size = size;
    block->next = new_block;
}

// -------------------------------------------------------
// internal: merge adjacent free blocks (coalescing)
// -------------------------------------------------------
static void heap_coalesce(void) {
    heap_block_t* cur = heap_start;
    while (cur && cur->next) {
        if (cur->free && cur->next->free) {
            // merge cur and cur->next into one block
            cur->size += sizeof(heap_block_t) + cur->next->size;
            cur->next  = cur->next->next;
            // don't advance — check again in case next is also free
        } else {
            cur = cur->next;
        }
    }
}

// -------------------------------------------------------
// public: initialize heap with one frame
// -------------------------------------------------------
void heap_init(void) {
    heap_start = heap_grow(0);   // grab one frame to start
    if (!heap_start) {
        terminal_writestring("HEAP: init failed!\n");
    }
}

// -------------------------------------------------------
// public: allocate size bytes
// -------------------------------------------------------
void* kmalloc(size_t size) {
    if (size == 0) return 0;

    // align size to 4 bytes — keeps things tidy
    size = (size + 3) & ~3;

    heap_block_t* cur = heap_start;

    while (cur) {
        if (cur->magic != HEAP_MAGIC) {
            terminal_writestring("HEAP: corruption detected!\n");
            return 0;
        }

        if (cur->free && cur->size >= size) {
            heap_split(cur, size);
            cur->free = 0;
            // return pointer AFTER the header
            return (void*)((uint8_t*)cur + sizeof(heap_block_t));
        }

        if (!cur->next) {
            // end of list — grow heap and link new block
            heap_block_t* new_block = heap_grow(size);
            if (!new_block) return 0;
            cur->next = new_block;
        }

        cur = cur->next;
    }

    return 0;
}

// -------------------------------------------------------
// public: free a previously allocated pointer
// -------------------------------------------------------
void kfree(void* ptr) {
    if (!ptr) return;

    // step back to find the header
    heap_block_t* block = (heap_block_t*)((uint8_t*)ptr
                           - sizeof(heap_block_t));

    if (block->magic != HEAP_MAGIC) {
        terminal_writestring("HEAP: kfree bad pointer!\n");
        return;
    }

    block->free = 1;
    heap_coalesce();   // merge adjacent free blocks
}

// -------------------------------------------------------
// debug: print every block in the heap
// -------------------------------------------------------
void heap_print(void) {
    heap_block_t* cur = heap_start;
    int i = 0;
    terminal_writestring("HEAP blocks:\n");
    while (cur) {
        terminal_writestring("  [");
        terminal_writehex(i++);
        terminal_writestring("] addr=");
        terminal_writehex((uint32_t)cur);
        terminal_writestring(" size=");
        terminal_writehex(cur->size);
        terminal_writestring(cur->free ? " FREE" : " USED");
        terminal_writestring("\n");
        cur = cur->next;
    }
}