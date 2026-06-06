#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

// syscall numbers
#define SYS_EXIT   1
#define SYS_WRITE  2
#define SYS_READ   3
#define SYS_SBRK   4

// register state pushed by syscall stub
typedef struct {
    uint32_t edi, esi, ebp, esp;
    uint32_t ebx, edx, ecx, eax;  // eax = syscall number
    uint32_t vector;               // always 0x80
    uint32_t err;                  // dummy error code
    // pushed by CPU on interrupt:
    uint32_t eip, cs, eflags;
    uint32_t useresp, ss;
} syscall_regs_t;

void syscall_init(void);
uint32_t syscall_handler(syscall_regs_t* regs);
void syscall_kbd_input(char c);

#endif