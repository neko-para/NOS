extern "C" void _start() {
    const char *str = "Hello world from Ring3!\n";
    asm volatile ( "movl %0, %%esi; movl $2, %%eax; int $0x80;" : "=m"(str) );
    asm volatile ( "movl $0, %eax; int $0x80;" );
}