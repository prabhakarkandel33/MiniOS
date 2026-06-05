#include "syscall.h"
#include "terminal.h"
#include "task.h"
#include <stdint.h>

// -------------------------------------------------------
// sys_exit — terminate current task
// -------------------------------------------------------
static uint32_t sys_exit(syscall_regs_t* regs) {
    (void)regs;
    terminal_writestring("process exited\n");
    terminate_task();
    return 0;  // never reached
}

// -------------------------------------------------------
// sys_write — write to terminal (fd 1 = stdout)
// ebx = fd, ecx = buffer address, edx = length
// -------------------------------------------------------
static uint32_t sys_write(syscall_regs_t* regs) {
    int fd       = (int)regs->ebx;
    char* buf    = (char*)regs->ecx;
    uint32_t len = regs->edx;

    if (fd == 1 || fd == 2) {  // stdout or stderr
        for (uint32_t i = 0; i < len; i++)
            terminal_putchar(buf[i]);
        return len;
    }
    return (uint32_t)-1;  // unsupported fd
}

// -------------------------------------------------------
// sys_read — read from keyboard (fd 0 = stdin)
// stub for now — returns 0
// -------------------------------------------------------
static uint32_t sys_read(syscall_regs_t* regs) {
    (void)regs;
    return 0;
}

// -------------------------------------------------------
// sys_sbrk — grow user heap
// stub for now — returns 0
// -------------------------------------------------------
static uint32_t sys_sbrk(syscall_regs_t* regs) {
    (void)regs;
    return 0;
}

// -------------------------------------------------------
// syscall dispatch table
// -------------------------------------------------------
typedef uint32_t (*syscall_fn_t)(syscall_regs_t*);

static syscall_fn_t syscall_table[] = {
    0,           // 0 — unused
    sys_exit,    // 1
    sys_write,   // 2
    sys_read,    // 3
    sys_sbrk,    // 4
};

#define SYSCALL_COUNT (sizeof(syscall_table) / sizeof(syscall_table[0]))

// -------------------------------------------------------
// main syscall handler — called from isr.s
// -------------------------------------------------------
uint32_t syscall_handler(syscall_regs_t* regs) {
    terminal_writestring("syscall! num=");
    terminal_writehex(regs->eax);
    terminal_writestring("\n");
    uint32_t num = regs->eax;

    if (num == 0 || num >= SYSCALL_COUNT || !syscall_table[num]) {
        terminal_writestring("unknown syscall: ");
        terminal_writehex(num);
        terminal_writestring("\n");
        return (uint32_t)-1;
    }

    return syscall_table[num](regs);
}

void syscall_init(void) {
    // nothing needed here yet
    // IDT entry is set in idt_init
}