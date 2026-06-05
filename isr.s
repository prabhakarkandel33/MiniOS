# isr.s — Interrupt Service Routine stubs (GAS / AT&T syntax)

.section .text
.extern exception_handler
.extern keyboard_handler

# -------------------------------------------------------
# Exception stubs — no error code
# Push dummy 0 so stack layout is uniform
# -------------------------------------------------------
.macro isr_no_err_stub num
isr_stub_\num:
    pushl $0            # dummy error code
    pushl $\num         # vector number
    jmp isr_common
.endm

# -------------------------------------------------------
# Exception stubs — CPU pushes error code automatically
# Stack on entry: [error_code] already pushed by CPU
# -------------------------------------------------------
.macro isr_err_stub num
isr_stub_\num:
    pushl $\num         # vector number (error code already on stack below)
    jmp isr_common
.endm

# -------------------------------------------------------
# Common exception handler path
# Stack on entry:
#   [vector]      <- ESP
#   [error_code]
#   [EIP]         <- pushed by CPU
#   [CS]
#   [EFLAGS]
#  (ring3 only):
#   [ESP_user]
#   [SS_user]
# -------------------------------------------------------
isr_common:
    pusha               # save EAX ECX EDX EBX ESP EBP ESI EDI

    # pass vector number as argument
    mov 32(%esp), %eax  # vector is at esp+32 (past 8 pushed regs)
    pushl %eax
    call exception_handler
    addl $4, %esp       # clean up vector arg

    popa                # restore registers
    addl $8, %esp       # discard vector + error code
    iret

# -------------------------------------------------------
# Define all 32 CPU exception handlers
# -------------------------------------------------------
isr_no_err_stub 0   # Divide-by-zero
isr_no_err_stub 1   # Debug
isr_no_err_stub 2   # Non-maskable interrupt
isr_no_err_stub 3   # Breakpoint
isr_no_err_stub 4   # Overflow
isr_no_err_stub 5   # Bound range exceeded
isr_no_err_stub 6   # Invalid opcode
isr_no_err_stub 7   # Device not available
isr_err_stub    8   # Double fault
isr_no_err_stub 9   # Coprocessor segment overrun
isr_err_stub    10  # Invalid TSS
isr_err_stub    11  # Segment not present
isr_err_stub    12  # Stack segment fault
isr_err_stub    13  # General protection fault
isr_err_stub    14  # Page fault
isr_no_err_stub 15  # Reserved
isr_no_err_stub 16  # x87 FPU error
isr_err_stub    17  # Alignment check
isr_no_err_stub 18  # Machine check
isr_no_err_stub 19  # SIMD FPU exception
isr_no_err_stub 20  # Virtualization exception
isr_no_err_stub 21  # Reserved
isr_no_err_stub 22  # Reserved
isr_no_err_stub 23  # Reserved
isr_no_err_stub 24  # Reserved
isr_no_err_stub 25  # Reserved
isr_no_err_stub 26  # Reserved
isr_no_err_stub 27  # Reserved
isr_no_err_stub 28  # Reserved
isr_no_err_stub 29  # Reserved
isr_err_stub    30  # Security exception
isr_no_err_stub 31  # Reserved

# -------------------------------------------------------
# ISR stub table — read by idt_init()
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
# pusha/popa to preserve registers across handler calls
# -------------------------------------------------------
.section .text
.extern irq_handler
.macro irq_stub_fn num handler
irq_stub_\num:
    pusha
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
#
# Stack on entry (from ring 3):
#   [SS_user]    <- pushed by CPU
#   [ESP_user]   <- pushed by CPU
#   [EFLAGS]     <- pushed by CPU
#   [CS_user]    <- pushed by CPU
#   [EIP_user]   <- pushed by CPU
#   <- ESP here
#
# We pusha to save all regs, pass pointer to saved regs
# to syscall_handler, then write EAX return value back
# into the saved EAX slot before restoring.
# -------------------------------------------------------
.section .text
.extern syscall_handler

syscall_stub:
    pusha                   # save all regs — EAX is at esp+28 after this

    push %esp               # arg: pointer to saved registers (syscall_regs_t*)
    call syscall_handler
    addl $4, %esp           # clean up arg

    movl %eax, 28(%esp)     # write return value into saved EAX slot
                            # pusha layout (low to high): EDI ESI EBP ESP EBX EDX ECX EAX
                            # so EAX is at esp+28

    popa                    # restore all regs (EAX now has return value)
    iret

.section .rodata
.global syscall_stub_addr
syscall_stub_addr:
.long syscall_stub