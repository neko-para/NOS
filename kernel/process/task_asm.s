.extern taskCurrent

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
