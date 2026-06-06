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
    mov $0x3F8, %edx
    mov $'U', %al
    outb %al, %dx  
    mov $2, %eax        # SYS_WRITE
    mov $1, %ebx        # stdout
    mov $msg, %ecx      # buffer — kernel address, accessible from user
    mov $17, %edx       # length
    int $0x80

    mov $1, %eax        # SYS_EXIT
    xor %ebx, %ebx
    int $0x80

    jmp user_entry

msg:
    .asciz "hello from ring3\n"   # spin in Ring 3

.global switch_to_user_mode_entry
.type switch_to_user_mode_entry, @function

switch_to_user_mode_entry:
    cli
    mov $0x23, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    mov 4(%esp), %eax    # user_stack
    mov 8(%esp), %ecx    # entry point

    pushl $0x23          # SS
    pushl %eax           # ESP
    pushfl
    popl %eax
    orl $0x200, %eax
    pushl %eax           # EFLAGS
    pushl $0x1B          # CS
    pushl %ecx           # EIP
    iretl

.global reset_and_enter_user
.type reset_and_enter_user, @function

# void reset_and_enter_user(uint32_t new_esp0, uint32_t user_stack, uint32_t entry)
reset_and_enter_user:
    mov 4(%esp), %eax    # new_esp0
    mov 8(%esp), %ebx    # user_stack  
    mov 12(%esp), %ecx   # entry

    # print 'R' to serial before iretl
    push %eax
    push %ebx  
    push %ecx
    mov $0x3F8, %edx
    mov $'R', %al
    outb %al, %dx
    pop %ecx
    pop %ebx
    pop %eax

    mov %eax, %esp
    cli
    push $0x23
    push %ebx
    pushfl
    pop %eax
    or $0x200, %eax
    push %eax
    push $0x1B
    push %ecx
    iretl