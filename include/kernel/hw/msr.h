#ifndef _HW_MSR_H_
#define _HW_MSR_H_
#include <inttypes.h>

#define IA32_APIC_BASE 0x0000001B

void 
__write_msr(uint32_t address, uint32_t lo, uint32_t hi);

void 
__read_msr(uint32_t address, uint32_t *lo, uint32_t *hi);

#endif // _HW_MSR_H_
