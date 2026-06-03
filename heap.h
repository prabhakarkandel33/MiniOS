#ifndef HEAP_H
#define HEAP_H

#include <stdint.h>
#include <stddef.h>

void  heap_init(void);
void* kmalloc(size_t size);
void  kfree(void* ptr);
void  heap_print(void);   // debug — print every block
uint32_t heap_get_start(void);


#endif