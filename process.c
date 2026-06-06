#include "process.h"
#include "paging.h"
#include "pmm.h"
#include "heap.h"
#include "task.h"
#include "tss.h"
#include "terminal.h"
#include <stdint.h>
#include "elf.h"
#include "ramfs.h"

static uint32_t next_pid = 1;

extern process_t* current_process;
// per-ELF launch args — heap allocated, set before task creation
typedef struct {
    uint32_t stack;
    uint32_t entry;
    process_t* proc;
} elf_args_t;

static elf_args_t* pending_elf_args = 0;

// forward declarations
void process_elf_entry(void);
void process_user_entry(void);

// -------------------------------------------------------
// user entry — called as kernel task, switches to ring 3
// used by process_create (the old path)
// -------------------------------------------------------
void process_user_entry(void) {
    extern void switch_to_user_mode(uint32_t user_stack);
    // stack is stored in pending_elf_args when called from process_create
    // but process_create no longer uses this path — kept for compatibility
    if (pending_elf_args) {
        switch_to_user_mode(pending_elf_args->stack);
    }
}

// -------------------------------------------------------
// ELF entry — called as kernel task, switches to ring 3
// reads stack+entry from pending_elf_args
// -------------------------------------------------------
void process_elf_entry(void) {
    extern void reset_and_enter_user(uint32_t, uint32_t, uint32_t);

    uint32_t user_stack  = pending_elf_args->stack;
    uint32_t entry       = pending_elf_args->entry;
    uint32_t stack_top   = (uint32_t)current_task->esp0;
    current_process      = pending_elf_args->proc;

    tss_set_kernel_stack(stack_top);
    reset_and_enter_user(stack_top, user_stack, entry);
}
// -------------------------------------------------------
// process_create — create a user process (legacy path)
// -------------------------------------------------------
process_t* process_create(void (*entry)(void), const char* name) {
    (void)entry;

    process_t* proc = (process_t*)kmalloc(sizeof(process_t));
    if (!proc) return 0;

    uint32_t dir_phys = 0;
    uint32_t* dir = paging_create_user_directory(&dir_phys);
    if (!dir) { kfree(proc); return 0; }

    proc->page_dir      = dir;
    proc->page_dir_phys = dir_phys;
    proc->id            = next_pid++;

    pageframe_t stack_frame = pmm_alloc_frame();
    if (stack_frame == PMM_ERROR) { kfree(proc); return 0; }
    paging_map_in(dir, USER_STACK_VIRT, stack_frame);
    proc->user_stack = USER_STACK_VIRT + USER_STACK_SIZE;

    elf_args_t* args = (elf_args_t*)kmalloc(sizeof(elf_args_t));
    if (!args) { kfree(proc); return 0; }
    args->stack = proc->user_stack;
    args->entry = 0;
    pending_elf_args = args;

    proc->task = create_kernel_task(process_user_entry, name);
    if (!proc->task) { kfree(args); kfree(proc); return 0; }
   proc->task->cr3 = (void*)proc->page_dir_phys;

    // terminal_writestring("page_dir_phys=");
    // terminal_writehex(proc->page_dir_phys);
    // terminal_writestring("\n");
    // terminal_writestring("task->cr3=");
    // terminal_writehex((uint32_t)proc->task->cr3);
    // terminal_writestring("\n");
}

// -------------------------------------------------------
// process_destroy
// -------------------------------------------------------
void process_destroy(process_t* proc) {
    if (!proc) return;
    terminate_task();
    kfree(proc);
}

// -------------------------------------------------------
// process_create_from_elf — load and run an ELF binary
// -------------------------------------------------------
process_t* process_create_from_elf(const char* filename) {
    // step 1 — find file in ramfs
    ramfs_node_t* node = ramfs_find(filename);
    if (!node) {
        terminal_writestring("exec: file not found: ");
        terminal_writestring(filename);
        terminal_writestring("\n");
        return 0;
    }

    // step 2 — validate ELF
    if (!elf_check_supported((Elf32_Ehdr*)node->data)) {
        terminal_writestring("exec: not a valid ELF\n");
        return 0;
    }

    // step 3 — allocate process struct and page directory
    process_t* proc = (process_t*)kmalloc(sizeof(process_t));
    if (!proc) return 0;

    uint32_t dir_phys = 0;
    uint32_t* dir = paging_create_user_directory(&dir_phys);
    if (!dir) { kfree(proc); return 0; }

    proc->page_dir      = dir;
    proc->page_dir_phys = dir_phys;
    proc->id            = next_pid++;

    // step 4 — allocate and map user stack
    pageframe_t stack_frame = pmm_alloc_frame();
    if (stack_frame == PMM_ERROR) { kfree(proc); return 0; }
    paging_map_in(dir, USER_STACK_VIRT, stack_frame);

    proc->user_stack = USER_STACK_VIRT + USER_STACK_SIZE;

    // step 5 — load ELF PT_LOAD segments into user page directory
    Elf32_Ehdr* hdr  = (Elf32_Ehdr*)node->data;
    Elf32_Phdr* phdr = (Elf32_Phdr*)((uint32_t)hdr + hdr->e_phoff);

    for (int i = 0; i < hdr->e_phnum; i++) {
        if (phdr[i].p_type != PT_LOAD) continue;

        uint32_t vaddr  = phdr[i].p_vaddr;
        uint32_t filesz = phdr[i].p_filesz;
        uint32_t memsz  = phdr[i].p_memsz;
        uint8_t* src    = (uint8_t*)hdr + phdr[i].p_offset;

        uint32_t pages = (memsz + PAGE_SIZE - 1) / PAGE_SIZE;

        for (uint32_t p = 0; p < pages; p++) {
            pageframe_t frame = pmm_alloc_frame();
            if (frame == PMM_ERROR) { kfree(proc); return 0; }

            uint32_t page_vaddr = vaddr + p * PAGE_SIZE;

            // map into user directory so CPU can access it at runtime
            paging_map_in(dir, page_vaddr, frame);

            // also map into current kernel address space so we can write to it now
            paging_map(page_vaddr, frame);

            // copy file data or zero BSS
            uint32_t offset   = p * PAGE_SIZE;
            uint32_t to_copy  = PAGE_SIZE;
            uint8_t* dst      = (uint8_t*)page_vaddr;

            if (offset < filesz) {
                if (offset + to_copy > filesz)
                    to_copy = filesz - offset;
                for (uint32_t b = 0; b < to_copy; b++)
                    dst[b] = src[offset + b];
                for (uint32_t b = to_copy; b < PAGE_SIZE; b++)
                    dst[b] = 0;
            } else {
                // BSS — zero entire page
                for (uint32_t b = 0; b < PAGE_SIZE; b++)
                    dst[b] = 0;
            }
        }
    }

    // step 6 — set up args and create kernel task
    uint32_t entry = hdr->e_entry;

    elf_args_t* args = (elf_args_t*)kmalloc(sizeof(elf_args_t));
    if (!args) { kfree(proc); return 0; }
    args->stack = proc->user_stack;
    args->entry = entry;
    args->proc  = proc;
    pending_elf_args = args;   // must be set before task runs


    proc->task = create_kernel_task(process_elf_entry, filename);
    if (!proc->task) { kfree(args); kfree(proc); return 0; }
    proc->task->cr3 = (void*)proc->page_dir_phys;

    terminal_writestring("exec: loaded ");
    terminal_writestring(filename);
    // terminal_writestring(" entry=");
    // terminal_writehex(entry);
    terminal_writestring("\n");


    // debug — verify mappings in user directory
    // terminal_writestring("dir[0x200000>>22]=");
    // terminal_writehex(dir[0x200000 >> 22]);  // slot 0
    // terminal_writestring("\n");
    // terminal_writestring("dir[0x800000>>22]=");
    // terminal_writehex(dir[0x800000 >> 22]);  // slot 2
    // terminal_writestring("\n");

    // also check the page table entry for 0x00200000
    // if (dir[0x200000 >> 22] & 1) {
    //     uint32_t* pt = (uint32_t*)((dir[0x200000 >> 22] & ~0xFFF) + 0xC0000000);
    //     terminal_writestring("pte[0x200000]=");
    //     terminal_writehex(pt[(0x200000 >> 12) & 0x3FF]);
    //     terminal_writestring("\n");
    // }
    // if (dir[0x800000 >> 22] & 1) {
    //     uint32_t* pt = (uint32_t*)((dir[0x800000 >> 22] & ~0xFFF) + 0xC0000000);
    //     terminal_writestring("pte[0x800000]=");
    //     terminal_writehex(pt[(0x800000 >> 12) & 0x3FF]);
    //     terminal_writestring("\n");
    // }

    return proc;
}

void process_wait(process_t* proc) {
    if (!proc) return;
    // store current task as the waiter
    proc->waiting_task = current_task;
    // block until child exits
    block_task(TASK_BLOCKED);
    // when we return here the child has exited
}