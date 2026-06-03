ENTRY (_start)

SECTIONS
{
    . = 0x00100000;

    _kernel_start = .;
    .multiboot.text : { *(.multiboot.text) }
    .multiboot.data : { *(.multiboot.data) }

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