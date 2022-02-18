.extern syscallHandler

.global isrHandler128
isrHandler128:
    pushl %eax
    pushl %gs
    pushl %fs
    pushl %es
    pushl %ds
    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %edx
    pushl %ecx
    pushl %ebx
    pushl %esp
    call syscallHandler
    addl $4, %esp
    popl %ebx
    popl %ecx
    popl %edx
    popl %esi
    popl %edi
    popl %ebp
    popl %ds
    popl %es
    popl %fs
    popl %gs
    addl $4, %esp
    iret
