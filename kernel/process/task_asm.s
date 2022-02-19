.extern taskCurrent
.extern sysTss

.global switchTask
switchTask:
    pushl %ebx
    pushl %esi
    pushl %edi
    pushl %ebp

    movl (taskCurrent), %edi
    movl %esp, (%edi)

    movl 20(%esp), %esi
    movl %esi, (taskCurrent)

    movl 0(%esi), %esp
    movl 20(%esi), %eax
    movl %eax, 4 + sysTss
    movl 4(%esi), %eax

    movl %cr3, %ecx
    cmpl %ecx, %eax
    je .doneVAS
    movl %eax, %cr3
.doneVAS:
    popl %ebp
    popl %edi
    popl %esi
    popl %ebx

    ret

.global switchRing3
switchRing3:
    movw $0x23, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    movl 4(%esp), %eax
    movl %eax, %esp
    iret