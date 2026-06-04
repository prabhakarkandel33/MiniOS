.section .text
.global switch_to_user_mode
.type switch_to_user_mode, @function

# void switch_to_user_mode(uint32_t user_stack)
# argument: user stack pointer passed on stack at 4(%esp)

switch_to_user_mode:
    cli

    # load user data segments
    mov $0x23, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    # get user stack from argument
    mov 4(%esp), %eax      # eax = user_stack pointer

    # build iret frame
    pushl $0x23            # SS  — user data
    pushl %eax             # ESP — user stack
    pushfl
    popl %eax
    orl $0x200, %eax       # set IF
    pushl %eax             # EFLAGS
    pushl $0x1B            # CS  — user code
    pushl $user_entry      # EIP
    iretl

user_entry:
    jmp user_entry         # spin in Ring 3