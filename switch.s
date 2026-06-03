# void switch_to_task(tcb_t* next)
#
# Stack at entry:
#   esp+0  = return address (pushed by call)
#   esp+4  = next task TCB pointer (argument)
#
# TCB layout (must match task.h):
#   offset 0  = esp
#   offset 4  = esp0
#   offset 8  = cr3

.section .text
.global switch_to_task
.type switch_to_task, @function

switch_to_task:
    # save callee-saved registers only
    pushl %ebp
    pushl %ebx
    pushl %esi
    pushl %edi

    # save current esp
    movl current_task, %eax
    movl %esp, (%eax)

    # load next
    movl 20(%esp), %esi
    movl %esi, current_task

    # switch stack
    movl (%esi), %esp

    # restore next task's registers
    popl %edi
    popl %esi
    popl %ebx
    popl %ebp

    ret                        # jumps to next task's saved EIP