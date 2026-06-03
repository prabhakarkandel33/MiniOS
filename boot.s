.set ALIGN,    1<<0
.set MEMINFO,  1<<1
.set FLAGS,    ALIGN | MEMINFO
.set MAGIC,    0x1BADB002
.set CHECKSUM, -(MAGIC + FLAGS)

.section .multiboot.data, "awx"
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

.section .bootstrap_stack, "aw", @nobits
stack_bottom:
.skip 16384
stack_top:

.section .bss, "aw", @nobits
.align 4096
boot_page_directory:
    .skip 4096
boot_page_table1:
    .skip 4096

.section .multiboot.text, "a"
.global _start
.type _start, @function
_start:
    # zero boot_page_directory
    movl $(boot_page_directory - 0xC0000000), %edi
    movl $1024, %ecx
    xorl %eax, %eax
    rep stosl

    # zero boot_page_table1
    movl $(boot_page_table1 - 0xC0000000), %edi
    movl $1024, %ecx
    xorl %eax, %eax
    rep stosl

    # fill page table starting at entry 256
    movl $(boot_page_table1 - 0xC0000000 + 256 * 4), %edi
    movl $0x00100000, %esi
    movl $767, %ecx

1:
    cmpl $_kernel_start, %esi
    jl 2f
    cmpl $(_kernel_end - 0xC0000000), %esi
    jge 3f

    movl %esi, %edx
    orl  $0x003, %edx
    movl %edx, (%edi)

2:
    addl $4096, %esi
    addl $4,    %edi
    loop 1b

3:
    # map VGA buffer to slot 1023 → virtual 0xC03FF000
    movl $(0x000B8000 | 0x003), boot_page_table1 - 0xC0000000 + 1023 * 4

    # identity map slot 0
    movl $(boot_page_table1 - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 0

    # higher half slot 768
    movl $(boot_page_table1 - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 768 * 4

    # load CR3
    movl $(boot_page_directory - 0xC0000000), %ecx
    movl %ecx, %cr3

    # enable paging + WP
    movl %cr0, %ecx
    orl  $0x80010000, %ecx
    movl %ecx, %cr0

    # jump to higher half
    lea  4f, %ecx
    jmp  *%ecx

.section .text
4:
    # remove identity map
    movl $0, boot_page_directory + 0
    movl %cr3, %ecx
    movl %ecx, %cr3

    mov $stack_top, %esp
    call kernel_main

    cli
5:
    hlt
    jmp 5b