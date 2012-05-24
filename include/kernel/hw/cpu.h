#ifndef _HW_CPU_H_
#define _HW_CPU_H_

#ifndef ASM_SOURCE
#include <kernel/types.h>

struct cpu_context
{
    uint16_t    __padding;
    
    uint16_t    gs;
    uint16_t    fs;
    uint16_t    es;
    uint16_t    ds;
    uint16_t    ss;
    uint32_t    edx;
    uint32_t    ecx;
    uint32_t    ebx;
    uint32_t    eax;
    uint32_t    esi;
    uint32_t    edi;
    uint32_t    ebp;
    uint32_t    error;
    uint32_t    eip;
    uint32_t    cs;
    uint32_t    eflags;
} __pack;


uint32_t 
cpuid_supported(void);
#endif

#endif // _HW_CPU_H_
