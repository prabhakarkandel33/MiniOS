#ifndef TASK_H
#define TASK_H


#include <stdint.h>
#include <stddef.h>

// Task states
#define TASK_RUNNING     0
#define TASK_READY       1
#define TASK_BLOCKED     2
#define TASK_SLEEPING    3
#define TASK_TERMINATED  4
#define TASK_PAUSED      5

#define KERNEL_STACK_SIZE 2048

#define TIME_SLICE_MS 50
extern volatile uint32_t time_slice_remaining;

typedef struct tcb {
    void*        esp;        // saved kernel stack pointer — MUST be first field
    void*        esp0;       // top of kernel stack (for TSS later)
    void*        cr3;        // page directory physical address
    struct tcb*  next;       // next task in whatever list this task is on
    uint8_t      state;      // current task state
    uint32_t     id;         // task ID
    const char*  name;       // task name (for debugging)
    uint32_t time_used;
    uint32_t last_tick;
} tcb_t;

// scheduler globals
extern tcb_t* current_task;

// functions
void  multitasking_init(void);
tcb_t* create_kernel_task(void (*entry)(void), const char* name);
void  schedule(void);
void  block_task(uint8_t reason);
void  unblock_task(tcb_t* task);
void  terminate_task(void);
void  switch_to_task(tcb_t* next);   // implemented in switch.s

// locking
void lock_scheduler(void);
void unlock_scheduler(void);
void lock_tasks(void);
void unlock_tasks(void);

extern tcb_t* idle_task;
void idle_task_fn(void);
void task_print_stats(void);

extern tcb_t* cleaner_task;
void cleaner_fn(void);
void terminate_task(void);
#endif