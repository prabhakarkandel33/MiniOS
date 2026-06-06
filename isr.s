# isr.s — Interrupt Service Routine stubs (GAS / AT&T syntax)

.section .text
.extern exception_handler
.extern keyboard_handler

# -------------------------------------------------------
# Exception stubs — no error code
# -------------------------------------------------------
.macro isr_no_err_stub num
isr_stub_\num:
    pushl $0
    pushl $\num
    jmp isr_common
.endm

# -------------------------------------------------------
# Exception stubs — CPU pushes error code
# -------------------------------------------------------
.macro isr_err_stub num
isr_stub_\num:
    pushl $\num
    jmp isr_common
.endm

# -------------------------------------------------------
# Common exception handler path
# -------------------------------------------------------
isr_common:
    pusha
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    mov 32(%esp), %eax
    pushl %eax
    call exception_handler
    addl $4, %esp

    popa
    addl $8, %esp
    iret

# -------------------------------------------------------
# All 32 CPU exception handlers
# -------------------------------------------------------
isr_no_err_stub 0
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
isr_err_stub    8
isr_no_err_stub 9
isr_err_stub    10
isr_err_stub    11
isr_err_stub    12
isr_err_stub    13
isr_err_stub    14
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub    17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_no_err_stub 21
isr_no_err_stub 22
isr_no_err_stub 23
isr_no_err_stub 24
isr_no_err_stub 25
isr_no_err_stub 26
isr_no_err_stub 27
isr_no_err_stub 28
isr_no_err_stub 29
isr_err_stub    30
isr_no_err_stub 31

# -------------------------------------------------------
# ISR stub table
# -------------------------------------------------------
.section .rodata
.global isr_stub_table
isr_stub_table:
.long isr_stub_0,  isr_stub_1,  isr_stub_2,  isr_stub_3
.long isr_stub_4,  isr_stub_5,  isr_stub_6,  isr_stub_7
.long isr_stub_8,  isr_stub_9,  isr_stub_10, isr_stub_11
.long isr_stub_12, isr_stub_13, isr_stub_14, isr_stub_15
.long isr_stub_16, isr_stub_17, isr_stub_18, isr_stub_19
.long isr_stub_20, isr_stub_21, isr_stub_22, isr_stub_23
.long isr_stub_24, isr_stub_25, isr_stub_26, isr_stub_27
.long isr_stub_28, isr_stub_29, isr_stub_30, isr_stub_31

# -------------------------------------------------------
# IRQ stubs — hardware interrupts (vectors 32-47)
# -------------------------------------------------------
.section .text
.extern irq_handler

.macro irq_stub_fn num handler
irq_stub_\num:
    pusha
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    call \handler
    popa
    iret
.endm

irq_stub_fn 0  irq_handler
irq_stub_fn 1  keyboard_handler
irq_stub_fn 2  irq_handler
irq_stub_fn 3  irq_handler
irq_stub_fn 4  irq_handler
irq_stub_fn 5  irq_handler
irq_stub_fn 6  irq_handler
irq_stub_fn 7  irq_handler
irq_stub_fn 8  irq_handler
irq_stub_fn 9  irq_handler
irq_stub_fn 10 irq_handler
irq_stub_fn 11 irq_handler
irq_stub_fn 12 irq_handler
irq_stub_fn 13 irq_handler
irq_stub_fn 14 irq_handler
irq_stub_fn 15 irq_handler

# -------------------------------------------------------
# IRQ stub table
# -------------------------------------------------------
.section .rodata
.global irq_stub_table
irq_stub_table:
.long irq_stub_0,  irq_stub_1,  irq_stub_2,  irq_stub_3
.long irq_stub_4,  irq_stub_5,  irq_stub_6,  irq_stub_7
.long irq_stub_8,  irq_stub_9,  irq_stub_10, irq_stub_11
.long irq_stub_12, irq_stub_13, irq_stub_14, irq_stub_15

# -------------------------------------------------------
# Syscall stub — vector 0x80
# -------------------------------------------------------
.section .text
.extern syscall_handler

syscall_stub:
    pusha
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    push %esp
    call syscall_handler
    addl $4, %esp

    movl %eax, 28(%esp)

    popa
    iret

.section .rodata
.global syscall_stub_addr
syscall_stub_addr:
.long syscall_stub