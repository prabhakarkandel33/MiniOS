// hello_user.c — compiled as a standalone i686 ELF
// compile: i686-elf-gcc -ffreestanding -nostdlib -O2 -o hello_user.elf hello_user.c

void _start(void) {
    // syscall write(1, "Hello from user!\n", 17)
    __asm__ volatile (
        "mov $2, %%eax\n"    // SYS_WRITE
        "mov $1, %%ebx\n"    // stdout
        "mov %0, %%ecx\n"    // buffer
        "mov $17, %%edx\n"   // length
        "int $0x80\n"
        :
        : "r"("Hello from user!\n")
        : "eax", "ebx", "ecx", "edx"
    );

    __asm__ volatile (
        "mov $1, %%eax\n"    // SYS_EXIT
        "xor %%ebx, %%ebx\n"
        "int $0x80\n"
        ::: "eax", "ebx"
    );
}