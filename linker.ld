ENTRY (_start)

SECTIONS
{
    . = 0x00100000;

    /* multiboot header and early boot code stay at physical 1MB */
    _kernel_start = .;
    .multiboot.data : { *(.multiboot.data) }
    .multiboot.text : { *(.multiboot.text) }

    /* everything else is virtually at 3GB but physically at 1MB */
    . += 0xC0000000;

    .text ALIGN(4K) : AT(ADDR(.text) - 0xC0000000)
    {
        *(.text)
    }

    .rodata ALIGN(4K) : AT(ADDR(.rodata) - 0xC0000000)
    {
        *(.rodata)
    }

    .data ALIGN(4K) : AT(ADDR(.data) - 0xC0000000)
    {
        *(.data)
    }

    .bss ALIGN(4K) : AT(ADDR(.bss) - 0xC0000000)
    {
        *(COMMON)
        *(.bss)
        *(.bootstrap_stack)
        endkernel = .;
    }

    _kernel_end = .;
}