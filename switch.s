.section .text
.global switch_to_task
.type switch_to_task, @function

switch_to_task:
    pushl %ebp
    pushl %ebx
    pushl %esi
    pushl %edi

    # save current esp
    movl current_task, %eax
    movl %esp, (%eax)

    # load next task
    movl 20(%esp), %esi
    movl %esi, current_task

    # switch stack
    movl (%esi), %esp

    # load next task's CR3 (offset 8 in TCB)
    movl 8(%esi), %eax
    movl %cr3, %ecx
    cmpl %eax, %ecx          # skip reload if same page directory
    je .Lno_cr3_reload
    movl %eax, %cr3
.Lno_cr3_reload:

    popl %edi
    popl %esi
    popl %ebx
    popl %ebp

    ret