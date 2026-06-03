# isr.s — Interrupt Service Routine stubs (GAS / AT&T syntax)

.section .text
.extern exception_handler
.extern keyboard_handler


# Macro for exceptions that push NO error code
# The CPU doesn't push one, so we push a dummy 0 to keep the stack uniform
.macro isr_no_err_stub num
isr_stub_\num:
    pusha
    pushl $\num         # push vector as argument RIGHT before call
    call exception_handler
    addl $4, %esp
    popa
    iret
.endm


# Macro for exceptions that DO push an error code automatically
.macro isr_err_stub num
isr_stub_\num:
    addl $4, %esp       # discard CPU error code (don't need it yet)
    pusha
    pushl $\num
    call exception_handler
    addl $4, %esp
    popa
    iret
.endm

# --- Define all 32 CPU exception handlers ---
# Which ones push an error code is defined by Intel's spec — not your choice.
# Vectors 8, 10, 11, 12, 13, 14, 17, 30 push error codes. The rest don't.

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
isr_err_stub    13  # General protection fault  ← you'll see this one a lot
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

# --- The stub table ---
# An array of 32 function pointers (4 bytes each with .long).
# idt_init() in C will read this to fill the IDT.

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

# IRQ stubs — hardware interrupts (vectors 32–47 after PIC remap)

.section .text
.extern irq_handler
.macro irq_stub_fn num handler
irq_stub_\num:
    call \handler
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

.section .rodata 
.global irq_stub_table
irq_stub_table:
.long irq_stub_0,  irq_stub_1,  irq_stub_2,  irq_stub_3
.long irq_stub_4,  irq_stub_5,  irq_stub_6,  irq_stub_7
.long irq_stub_8,  irq_stub_9,  irq_stub_10, irq_stub_11
.long irq_stub_12, irq_stub_13, irq_stub_14, irq_stub_15