.section .text
.global switch_to_user_mode
.type switch_to_user_mode, @function

switch_to_user_mode:
    cli

    # load user data segments
    mov $0x23, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    # save current esp
    mov %esp, %eax

    # build iret frame
    pushl $0x23        # SS — user data
    pushl %eax         # ESP
    pushfl             # EFLAGS
    popl %eax
    orl $0x200, %eax   # set IF bit
    pushl %eax         # EFLAGS
    pushl $0x1B        # CS — user code
    pushl $user_entry  # EIP
    iretl

user_entry:
    # now in Ring 3
    # spin forever for now
    jmp user_entry