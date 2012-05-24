#define _NO_EXTERN_APIC_ADDR
#include <kernel/hw/apic.h>
#include <kernel/hw/msr.h>

uint32_t* APIC_BASE_ADDRESS;

void 
init_apic(uint32_t relocation)
{
    uint32_t hi, lo;
   
    __read_msr(IA32_APIC_BASE, &lo, &hi);
    
    if (relocation != 0)
    {
        lo &= 0x00000FFF;
        lo |= (relocation & 0xFFFFF000);
        
        __write_msr(IA32_APIC_BASE, lo, hi);
        APIC_BASE_ADDRESS = (uint32_t*)relocation;   
    }
    else
        // Extract the apic base address
        APIC_BASE_ADDRESS = (uint32_t*)(lo & 0xFFFFF000);  
}


uint32_t 
read_lapic_reg(uint16_t reg)
{
    uint32_t r;
    
    // 32-bit read on a 128-bit aligned boundary
    r = *(uint32_t *)((uint8_t*)APIC_BASE_ADDRESS + reg);
    
    return r;
}


void 
write_lapic_reg(uint16_t reg, uint32_t value)
{
    *(uint32_t *)((uint8_t*)APIC_BASE_ADDRESS + reg) = value;
}
