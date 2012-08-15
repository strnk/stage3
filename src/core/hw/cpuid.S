#include <kernel/hw/cpu.h>

.file "cpuid.S"

.global cpuid_supported

.text

cpuid_supported:
    pushf
    popq    %rax
    movq    %rax, %rcx
    xorq    $0x200000, %rax 
    pushq   %rax
    popf
    pushf
    popq    %rax
    xor     %rcx, %rax
    shr     $21, %rax
    and     $1, %rax
    pushq   %rcx
    popf
    ret
