#include <kernel/hw/cpu.h>

.file "cpuid.S"

.global cpuid_supported

.text

cpuid_supported:
    pushf
    popl    %eax
    movl    %eax, %ecx
    xorl    $0x200000, %eax 
    pushl   %eax
    popf
    pushf
    popl    %eax
    xor     %ecx, %eax
    shr     $21, %eax
    and     $1, %eax
    pushl   %ecx
    popf
    ret
