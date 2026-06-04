// test_module.c
void module_main(void) {
    // can't call kernel functions yet
    // just return so we can verify execution
    volatile int x = 42;
    (void)x;
}