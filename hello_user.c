void _start(void) {
    // write "Hi!\n" using only stack — no .rodata reference
    char msg[5];
    msg[0] = 'H';
    msg[1] = 'i';
    msg[2] = '!';
    msg[3] = '\n';
    msg[4] = 0;

    __asm__ volatile (
        "mov $2, %%eax\n"     // SYS_WRITE
        "mov $1, %%ebx\n"     // fd=stdout
        "lea %0, %%ecx\n"     // buf = &msg (on stack, always valid)
        "mov $4, %%edx\n"     // len=4
        "int $0x80\n"
        : : "m"(msg[0])
        : "eax", "ebx", "ecx", "edx"
    );

    __asm__ volatile (
        "mov $1, %%eax\n"     // SYS_EXIT
        "xor %%ebx, %%ebx\n"
        "int $0x80\n"
        ::: "eax", "ebx"
    );
}