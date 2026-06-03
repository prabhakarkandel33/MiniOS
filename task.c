#include "task.h"
#include "heap.h"
#include "paging.h"
#include "terminal.h"
#include "pic.h"
#include <stdint.h>

tcb_t* current_task = 0;
extern volatile uint32_t tick;

static void update_time_used(void);

// ready to run queue
static tcb_t* ready_first = 0;
static tcb_t* ready_last  = 0;

tcb_t* idle_task =0;
volatile uint32_t time_slice_remaining = 0;

tcb_t* cleaner_task = 0;
static tcb_t* terminated_list = 0;

static void cleanup_task(tcb_t* task){
    kfree((uint8_t*)task->esp0 - KERNEL_STACK_SIZE);
    kfree(task);
}
void cleaner_fn(void) {
    for (;;) {
        lock_tasks();
        while (terminated_list) {
            tcb_t* task = terminated_list;
            terminated_list = task->next;
            cleanup_task(task);
        }
        unlock_tasks();          // unlock BEFORE blocking
        block_task(TASK_PAUSED); // now safe to block
    }
}

void idle_task_fn(void){
    for(;;){
        __asm__ volatile("hlt");
    }
}

// task ID counter
static uint32_t next_id = 1;

// scheduler lock counter
static int irq_disable_count = 0;
static int postpone_count = 0;
static int task_switch_postponed = 0;


void do_task_switch(tcb_t* next){
    update_time_used();
    next->last_tick = tick;
    if (next == idle_task){
        time_slice_remaining = 0;
    }else {
        time_slice_remaining = TIME_SLICE_MS;
    }
    switch_to_task(next);
}

void task_print_stats(void) {
    terminal_writestring("current: ");
    terminal_writestring(current_task->name);
    terminal_writestring(" time=");
    terminal_writehex(current_task->time_used);
    terminal_writestring("\n");
}
// -------------------------------------------------------
// locking — disable/enable IRQs with nesting support
// -------------------------------------------------------
void lock_scheduler(void) {
    __asm__ volatile ("cli");
    irq_disable_count++;
}

void unlock_scheduler(void) {
    irq_disable_count--;
    if (irq_disable_count == 0)
        __asm__ volatile ("sti");
}

void lock_tasks(void){
    __asm__ volatile("cli");
    irq_disable_count++;
    postpone_count++;
}

void unlock_tasks(void) {
    postpone_count--;
    if (postpone_count == 0 && task_switch_postponed) {
        task_switch_postponed = 0;
        irq_disable_count--;
        if (irq_disable_count == 0)
            __asm__ volatile ("sti");
        schedule();    // schedule after sti is fine — it locks internally
        return;
    }
    irq_disable_count--;
    if (irq_disable_count == 0)
        __asm__ volatile ("sti");
}

// -------------------------------------------------------
// add task to end of ready queue
// -------------------------------------------------------
static void enqueue(tcb_t* task) {
    task->next = 0;
    task->state = TASK_READY;
    if (!ready_last) {
        ready_first = ready_last = task;
    } else {
        ready_last->next = task;
        ready_last = task;
    }
}

// -------------------------------------------------------
// remove task from front of ready queue
// -------------------------------------------------------
static tcb_t* dequeue(void) {
    if (!ready_first) return 0;
    tcb_t* task = ready_first;
    ready_first = task->next;
    if (!ready_first) ready_last = 0;
    task->next = 0;
    return task;
}

// -------------------------------------------------------
// initialize multitasking — call once from kernel_main
// wraps the currently running kernel into task 0
// -------------------------------------------------------
void multitasking_init(void) {
    tcb_t* task0 = (tcb_t*)kmalloc(sizeof(tcb_t));
    task0->esp   = 0;
    task0->esp0  = 0;
    task0->next  = 0;
    task0->state = TASK_RUNNING;
    task0->id    = next_id++;
    task0->name  = "kernel";
    task0->time_used = 0;
    task0->last_tick = 0;

    uint32_t cr3;
    __asm__ volatile ("mov %%cr3, %0" : "=r"(cr3));
    task0->cr3 = (void*)cr3;
    current_task = task0;

    // create idle task but DON'T enqueue it — scheduler uses it as fallback only
    tcb_t* task = (tcb_t*)kmalloc(sizeof(tcb_t));
    uint8_t* stack = (uint8_t*)kmalloc(KERNEL_STACK_SIZE);
    uint32_t* sp = (uint32_t*)(stack + KERNEL_STACK_SIZE);
    *--sp = (uint32_t)idle_task_fn;
    *--sp = 0;
    *--sp = 0;
    *--sp = 0;
    *--sp = 0;

    task->esp   = sp;
    task->esp0  = stack + KERNEL_STACK_SIZE;
    task->cr3   = (void*)cr3;
    task->next  = 0;
    task->state = TASK_READY;
    task->id    = next_id++;
    task->name  = "idle";
    idle_task   = task;
    task->time_used = 0;
    task->last_tick = 0;
    // intentionally NOT calling enqueue(task)
    cleaner_task = create_kernel_task(cleaner_fn, "cleaner");
}

// -------------------------------------------------------
// create a new kernel task
// entry = function to run
// -------------------------------------------------------
tcb_t* create_kernel_task(void (*entry)(void), const char* name) {
    // terminal_writestring("alloc tcb\n");
    tcb_t* task = (tcb_t*)kmalloc(sizeof(tcb_t));
    // terminal_writestring("alloc stack\n");
    uint8_t* stack = (uint8_t*)kmalloc(KERNEL_STACK_SIZE);
    
   

    uint32_t* sp = (uint32_t*)(stack + KERNEL_STACK_SIZE);

    *--sp = (uint32_t)entry;
    *--sp = 0;
    *--sp = 0;
    *--sp = 0;
    *--sp = 0;
   

    // get current CR3 — kernel tasks share the kernel page directory
    uint32_t cr3;
    __asm__ volatile ("mov %%cr3, %0" : "=r"(cr3));

    task->esp   = sp;
    task->esp0  = stack + KERNEL_STACK_SIZE;
    task->cr3   = (void*)cr3;
    task->next  = 0;
    task->state = TASK_READY;
    task->id    = next_id++;
    task->name  = name;
    task->time_used = 0;
    task->last_tick = 0;

    // add to ready queue
    lock_scheduler();
    enqueue(task);
    unlock_scheduler();

    return task;
}

// -------------------------------------------------------
// schedule — pick next task and switch to it
// -------------------------------------------------------
void schedule(void) {
    lock_scheduler();

    if (postpone_count > 0) {
        task_switch_postponed = 1;
        unlock_scheduler();
        return;
    }

    tcb_t* next = dequeue();
    if (!next) {
        // nothing ready — if current task is still running, keep it
        if (current_task->state == TASK_RUNNING) {
            unlock_scheduler();
            return;
        }
        // current task blocked and nothing ready — run idle
        next = idle_task;
    }

    // avoid giving idle task CPU when others are ready
    if (next == idle_task && ready_first != 0) {
        // put idle back and take the real task
        enqueue(next);
        next = dequeue();
    }

    if (current_task->state == TASK_RUNNING) {
        enqueue(current_task);
    }

    next->state = TASK_RUNNING;
    unlock_scheduler();
    do_task_switch(next);
}

// -------------------------------------------------------
// block current task
// -------------------------------------------------------
void block_task(uint8_t reason) {
    lock_scheduler();
    current_task->state = reason;
    tcb_t* next = dequeue();
    if (!next) next = idle_task;   // fallback to idle
    next->state = TASK_RUNNING;
    unlock_scheduler();
    do_task_switch(next);
}

// -------------------------------------------------------
// unblock a task — put it back on ready queue
// -------------------------------------------------------
void unblock_task(tcb_t* task) {
    lock_scheduler();
    enqueue(task);
    // preempt if idle task is currently running
    if (current_task == idle_task) {
        tcb_t* next = dequeue();
        if (next) {
            next->state = TASK_RUNNING;
            unlock_scheduler();
            do_task_switch(next);
            return;
        }
    }
    unlock_scheduler();
}
// -------------------------------------------------------
// terminate current task
// -------------------------------------------------------
void terminate_task(void) {
    lock_tasks();

    current_task->next = terminated_list;
    terminated_list = current_task;
    current_task->state = TASK_TERMINATED;

    if (cleaner_task && cleaner_task->state == TASK_PAUSED)
        unblock_task(cleaner_task);

    tcb_t* next = dequeue();
    if (!next) next = idle_task;
    next->state = TASK_RUNNING;
    unlock_tasks();
    do_task_switch(next);
    // never returns
}

static void update_time_used(void){
    if(!current_task) return;
    current_task->time_used += tick - current_task -> last_tick;
    current_task->last_tick=tick;
}

