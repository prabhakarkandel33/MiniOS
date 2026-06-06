//interactive more intresting test program to run in the OS
static void write_str(const char* s, int len) {
    __asm__ volatile (
        "int $0x80"
        :
        : "a"(2), "b"(1), "c"(s), "d"(len)
    );
}

static int read_line(char* buf, int max) {
    int n;
    __asm__ volatile (
        "int $0x80"
        : "=a"(n)
        : "a"(3), "b"(0), "c"(buf), "d"(max)
    );
    return n;
}

static void write_char(char c) {
    write_str(&c, 1);
}

static int str_len(const char* s) {
    int n = 0;
    while (s[n]) n++;
    return n;
}

void _start(void) {
    const char* welcome = "=== prabhakarOS user shell ===\n";
    write_str(welcome, str_len(welcome));

    char buf[64];
    while (1) {
        const char* prompt = "> ";
        write_str(prompt, 2);

        int n = read_line(buf, 63);
        buf[n] = '\0';

        // simple echo command
        if (n > 0 && buf[0] != '\n') {
            const char* echo = "you typed: ";
            write_str(echo, str_len(echo));
            write_str(buf, n);
        }

        // exit on 'q'
        if (buf[0] == 'q') break;
    }

    const char* bye = "goodbye!\n";
    write_str(bye, str_len(bye));

    __asm__ volatile ("int $0x80" : : "a"(1), "b"(0));
}