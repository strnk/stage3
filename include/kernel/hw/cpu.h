#ifndef _HW_CPU_H_
#define _HW_CPU_H_

#ifndef ASM_SOURCE
#include <kernel/types.h>

struct cpu_context
{
    uint32_t    __padding;
    
    uint16_t    gs;
    uint16_t    fs;
    uint64_t    rdx;
    uint64_t    rcx;
    uint64_t    rbx;
    uint64_t    rax;
    uint64_t    rsi;
    uint64_t    rdi;
    uint64_t    rbp;
    uint64_t    error;
    uint64_t    rip;
    uint16_t    cs;
    uint64_t    rflags;
} __pack;


uint32_t 
cpuid_supported(void);
#endif

#endif // _HW_CPU_H_
