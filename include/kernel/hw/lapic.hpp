#ifndef _HW_LAPIC_H_
#define _HW_LAPIC_H_

#include <stdlib.h>
#include <inttypes.h>
#include <kernel/types.h>
#include <kernel/hw/interrupts.hpp>
#include <kernel/hw/ioapic.hpp>

#define APIC_REG_ID         0x0020
#define APIC_REG_VERSION    0x0030
#define APIC_REG_TPR        0x0080
#define APIC_REG_APR        0x0090
#define APIC_REG_PPR        0x00A0
#define APIC_REG_EOI        0x00B0
#define APIC_REG_LDR        0x00D0
#define APIC_REG_DFR        0x00E0
#define APIC_REG_SPURIOUS   0x00F0

#define APIC_REG_ISR        0x0100
#define APIC_REG_TMR        0x0170
#define APIC_REG_IRR        0x0200

#define APIC_REG_LVT_TIMER  0x0320
#define APIC_REG_LVT_THERM  0x0330
#define APIC_REG_LVT_PERF   0x0340
#define APIC_REG_LVT_LINT0  0x0350
#define APIC_REG_LVT_LINT1  0x0360
#define APIC_REG_LVT_ERR    0x0370


namespace Stage3 {
namespace Interrupts {

    class APICManager : public Manager
    {
        uint32_t* APIC_BASE_ADDRESS;
        uint8_t ioapic_id;
        IOAPIC ioapic;
        
        struct _ioapic_map
        {
            bus_type_t bus_id;
            uint8_t bus_irq;
        } ioMap[24];
        
        public:
        APICManager();
        
        uint8_t getType();
        
        void init();
        void shutdown();
        
        void enable(bus_type_t bus, uint8_t irq);
        void disable(bus_type_t bus, uint8_t irq);
        
        void map(bus_type_t bus, uint8_t irq, uint8_t vector);
        
        void eoi(uint64_t vector);
        
        private:
        void _initLAPIC();
        uint32_t _readLAPIC(uint16_t reg);
        void _writeLAPIC(uint16_t reg, uint32_t value);
        int8_t _getIrqFromBus(bus_type_t bus_id, uint8_t bus_irq);
    };
    
}
}
void 
init_apic(phys_addr_t relocation);

uint32_t 
read_lapic_reg(uint16_t reg);

void 
write_lapic_reg(uint16_t address, uint32_t value);

#ifndef _NO_EXTERN_APIC_ADDR
extern uint32_t* APIC_BASE_ADDRESS;
#endif 

#endif // _HW_LAPIC_H_
