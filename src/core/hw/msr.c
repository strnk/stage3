#include <kernel/hw/msr.h>

void 
__write_msr(uint32_t address, uint32_t lo, uint32_t hi)
{
    asm volatile("wrmsr" :: "a"(lo), "d"(hi), "c"(address));
}

void 
__read_msr(uint32_t address, uint32_t *lo, uint32_t *hi)
{
    asm volatile("rdmsr" : "=a"(*lo), "=d"(*hi) : "c"(address));
}
