# boot.s — multiboot + higher half setup

.set ALIGN,    1<<0
.set MEMINFO,  1<<1
.set FLAGS,    ALIGN | MEMINFO
.set MAGIC,    0x1BADB002
.set CHECKSUM, -(MAGIC + FLAGS)

.section .multiboot.data, "aw"
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# 16KB stack in BSS
.section .bootstrap_stack, "aw", @nobits
stack_bottom:
.skip 16384
stack_top:

# page directory and one page table in BSS — aligned to 4KB
.section .bss, "aw", @nobits
.align 4096
boot_page_directory:
    .skip 4096
boot_page_table1:
    .skip 4096

# early boot code runs at physical addresses — goes in .multiboot.text
.section .multiboot.text, "a"
.global _start
.type _start, @function
_start:
    # edi = physical address of boot_page_table1
    movl $(boot_page_table1 - 0xC0000000), %edi
    movl $0, %esi          # esi = current physical address being mapped
    movl $1023, %ecx       # map 1023 pages (last one reserved for VGA)

1:
    # only map pages within the kernel image
    cmpl $_kernel_start, %esi
    jl 2f
    cmpl $(_kernel_end - 0xC0000000), %esi
    jge 3f

    # entry = physical address | present | writable
    movl %esi, %edx
    orl  $0x003, %edx
    movl %edx, (%edi)

2:
    addl $4096, %esi       # next physical page
    addl $4,    %edi       # next page table entry
    loop 1b

3:
    # map VGA buffer (0xB8000) to last entry of page table → virtual 0xC03FF000
    movl $(0x000B8000 | 0x003), boot_page_table1 - 0xC0000000 + 1023 * 4

    # put boot_page_table1 into directory slot 0   (identity map, 0x00000000)
    movl $(boot_page_table1 - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 0

    # put boot_page_table1 into directory slot 768 (higher half, 0xC0000000)
    movl $(boot_page_table1 - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 768 * 4

    # load page directory into CR3
    movl $(boot_page_directory - 0xC0000000), %ecx
    movl %ecx, %cr3

    # enable paging + write protect
    movl %cr0, %ecx
    orl  $0x80010000, %ecx
    movl %ecx, %cr0

    # absolute jump into higher half virtual address
    lea  4f, %ecx
    jmp  *%ecx

.section .text
4:
    # now running at 0xC0xxxxxx — remove identity mapping
    movl $0, boot_page_directory + 0
    movl %cr3, %ecx
    movl %ecx, %cr3        # flush TLB

    # set up stack using virtual address
    mov $stack_top, %esp

    call kernel_main

    cli
5:
    hlt
    jmp 5b