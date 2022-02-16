.set MAGIC, 0x1BADB002
.set FLAGS, 3

.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long -(MAGIC + FLAGS)

.section .bss
.global stack_top
.align 16
stack_bottom:
.skip 16384
stack_top:

.section .text
.global _start
_start:
    movl $stack_top, %esp
    pushl %ebx

    call kernel_main

    cli
1:
    hlt
    jmp 1b
