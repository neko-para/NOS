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

.gdtr:
.word 0
.long 0

.global setGdt
setGdt:
    movl 4(%esp), %eax
    movl %eax, (.gdtr + 2)
    mov 8(%esp), %ax
    dec %ax
    mov %ax, (.gdtr)
    lgdt (.gdtr)
    ret

.global reloadSegments
reloadSegments:
    ljmp $0x08, $.reload
.reload:
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    ret
