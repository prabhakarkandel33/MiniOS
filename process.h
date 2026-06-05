#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include "task.h"
#include "elf.h"
#include "ramfs.h"

typedef struct {
    tcb_t*    task;          // scheduler task for this process
    uint32_t* page_dir;      // virtual address of page directory
    uint32_t  page_dir_phys; // physical address (for CR3)
    uint32_t  user_stack;    // user stack top virtual address
    uint32_t  id;            // process ID
} process_t;

process_t* process_create(void (*entry)(void), const char* name);
void       process_destroy(process_t* proc);
process_t* process_create_from_elf(const char* filename);


#endif