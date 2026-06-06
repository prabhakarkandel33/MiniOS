#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include "task.h"
#include "elf.h"
#include "ramfs.h"

#define USER_STACK_VIRT  0x00800000
#define USER_STACK_SIZE  4096
#define USER_CODE_VIRT   0x00200000

typedef struct {
    tcb_t*    task;
    uint32_t* page_dir;
    uint32_t  page_dir_phys;
    uint32_t  user_stack;
    uint32_t  id;
    tcb_t*    waiting_task;
} process_t;

extern process_t* current_process;

process_t* process_create(void (*entry)(void), const char* name);
process_t* process_create_from_elf(const char* filename);
void       process_destroy(process_t* proc);
void       process_wait(process_t* proc);

#endif