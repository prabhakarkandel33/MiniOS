#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "task.h"

typedef struct {
    int     max_count;
    int     current_count;
    tcb_t*  first_waiting;
    tcb_t*  last_waiting;
} semaphore_t;

semaphore_t* semaphore_create(int max_count);
semaphore_t* mutex_create(void);
void semaphore_acquire(semaphore_t* s);
void semaphore_release(semaphore_t* s);
void mutex_acquire(semaphore_t* m);
void mutex_release(semaphore_t* m);

#endif