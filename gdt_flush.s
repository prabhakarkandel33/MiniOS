.global gdt_flush
.type gdt_flush, @function

gdt_flush:
    mov 4(%esp), %eax 
    lgdt (%eax) # load the gdt

    # Reload segment register with kernel data sector
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    # far jump to reload kernel code
    ljmp $0x08, $.flush

    .flush:
        ret
