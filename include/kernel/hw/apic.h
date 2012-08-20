#ifndef _HW_APIC_H_
#define _HW_APIC_H_

#include <stdlib.h>
#include <inttypes.h>
#include <kernel/types.h>

#define APIC_REG_ID         0x0020
#define APIC_REG_VERSION    0x0030
#define APIC_REG_TPR        0x0080
#define APIC_REG_APR        0x0090
#define APIC_REG_PPR        0x00A0
#define APIC_REG_EOI        0x00B0
#define APIC_REG_LDR        0x00D0
#define APIC_REG_DFR        0x00E0
#define APIC_REG_SPURIOUS   0x00F0

#define APIC_REG_LVT_TIMER  0x0320
#define APIC_REG_LVT_THERM  0x0330
#define APIC_REG_LVT_PERF   0x0340
#define APIC_REG_LVT_LINT0  0x0350
#define APIC_REG_LVT_LINT1  0x0360
#define APIC_REG_LVT_ERR    0x0370

void 
init_apic(phys_addr_t relocation);

uint32_t 
read_lapic_reg(uint16_t reg);

void 
write_lapic_reg(uint16_t address, uint32_t value);

#ifndef _NO_EXTERN_APIC_ADDR
extern uint32_t* APIC_BASE_ADDRESS;
#endif 

#endif // _HW_APIC_H_
