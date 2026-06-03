#include "semaphore.h"
#include "heap.h"
#include "terminal.h"

semaphore_t* semaphore_create(int max_count) {
    semaphore_t* s = (semaphore_t*)kmalloc(sizeof(semaphore_t));
    if (!s) return 0;
    s->max_count     = max_count;
    s->current_count = 0;
    s->first_waiting = 0;
    s->last_waiting  = 0;
    return s;
}

semaphore_t* mutex_create(void) {
    return semaphore_create(1);
}

void semaphore_acquire(semaphore_t* s) {
    lock_tasks();
    if (s->current_count < s->max_count) {
        s->current_count++;
    } else {
        // add to end of waiting list
        current_task->next = 0;
        if (!s->first_waiting) {
            s->first_waiting = current_task;
        } else {
            s->last_waiting->next = current_task;
        }
        s->last_waiting = current_task;
        block_task(TASK_BLOCKED);
    }
    unlock_tasks();
}

void semaphore_release(semaphore_t* s) {
    lock_tasks();
    if (s->first_waiting) {
        // wake first waiting task — count stays same
        tcb_t* task = s->first_waiting;
        s->first_waiting = task->next;
        if (!s->first_waiting) s->last_waiting = 0;
        unblock_task(task);
    } else {
        s->current_count--;
    }
    unlock_tasks();
}

void mutex_acquire(semaphore_t* m) { semaphore_acquire(m); }
void mutex_release(semaphore_t* m) { semaphore_release(m); }