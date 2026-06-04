#include "process.h"
#include "paging.h"
#include "pmm.h"
#include "heap.h"
#include "task.h"
#include "tss.h"
#include "terminal.h"
#include <stdint.h>

static uint32_t next_pid = 1;

process_t* process_create(void (*entry)(void), const char* name) {
    process_t* proc = (process_t*)kmalloc(sizeof(process_t));
    if (!proc) return 0;

    // step 1 — create page directory with kernel mappings
    uint32_t* dir = paging_create_user_directory();
    if (!dir) { kfree(proc); return 0; }

    proc->page_dir      = dir;
    proc->page_dir_phys = (uint32_t)dir - 0xC0000000;  // physical
    proc->id            = next_pid++;

    // step 2 — allocate and map user stack
    pageframe_t stack_frame = pmm_alloc_frame();
    if (stack_frame == PMM_ERROR) {
        kfree(proc); return 0;
    }
    paging_map_in(dir, USER_STACK_VIRT, stack_frame);
    proc->user_stack = USER_STACK_VIRT + USER_STACK_SIZE;

    // step 3 — create kernel task for this process
    // the task starts in kernel mode then switches to user mode
    proc->task = create_kernel_task(entry, name);
    if (!proc->task) { kfree(proc); return 0; }

    // step 4 — point task's CR3 to new page directory
    proc->task->cr3 = (void*)proc->page_dir_phys;

    terminal_writestring("process created: ");
    terminal_writestring(name);
    terminal_writestring(" pid=");
    terminal_writehex(proc->id);
    terminal_writestring("\n");

    return proc;
}

void process_destroy(process_t* proc) {
    if (!proc) return;
    terminate_task();
    kfree(proc);
}