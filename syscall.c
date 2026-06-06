#include "syscall.h"
#include "terminal.h"
#include "task.h"
#include "process.h"

#include <stdint.h>

process_t* current_process = 0;

// -------------------------------------------------------
// sys_exit — terminate current task
// -------------------------------------------------------
static uint32_t sys_exit(syscall_regs_t* regs) {
    (void)regs;
    // terminal_writestring("process exited\n");
    // terminal_writestring("current_process=");
    // terminal_writehex((uint32_t)current_process);
    // terminal_writestring("\n");
    // if (current_process) {
    //     terminal_writestring("waiting_task=");
    //     terminal_writehex((uint32_t)current_process->waiting_task);
    //     terminal_writestring("\n");
    // }
    if (current_process && current_process->waiting_task) {
        tcb_t* waiter = current_process->waiting_task;
        current_process->waiting_task = 0;
        // terminal_writestring("unblocking waiter\n");
        unblock_task(waiter);
    }
    terminate_task();
    return 0;
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



// simple keyboard buffer for user mode reads
static char kbd_buf[256];
static int  kbd_len = 0;
static int  kbd_ready = 0;

void syscall_kbd_input(char c) {
    if (kbd_len < 255) {
        kbd_buf[kbd_len++] = c;
        if (c == '\n') kbd_ready = 1;
    }
}


static uint32_t sys_read(syscall_regs_t* regs) {
    int fd       = (int)regs->ebx;
    char* buf    = (char*)regs->ecx;
    uint32_t len = regs->edx;

    if (fd != 0) return (uint32_t)-1;

    // spin until enter pressed
    while (!kbd_ready) {
        __asm__ volatile ("hlt");
    }

    uint32_t n = kbd_len < (int)len ? kbd_len : len;
    for (uint32_t i = 0; i < n; i++)
        buf[i] = kbd_buf[i];

    kbd_len   = 0;
    kbd_ready = 0;
    return n;
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
    // terminal_writestring("syscall! num=");
    // terminal_writehex(regs->eax);
    // terminal_writestring("\n");
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