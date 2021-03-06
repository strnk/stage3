#define _NO_EXTERN_APIC_ADDR
#include <kernel/hw/lapic.hpp>
#include <kernel/hw/paging.hpp>
#include <kernel/hw/pm_alloc.hpp>
#include <kernel/hw/mp.hpp>
#include <kernel/hw/msr.h>

#include <cstdio>

using namespace Stage3;
using namespace Stage3::Interrupts;

APICManager::APICManager()
{
}

uint8_t 
APICManager::getType()
{
    return 1;
}

void 
APICManager::init()
{
    ioapic_id = MP::getFirstIOAPIC();
    ioapic = Interrupts::IOAPIC(MP::getIOAPICaddr(ioapic_id));

    PhysicalMemoryAllocator::mark_reserved(
        MP::getIOAPICaddr(ioapic_id), MP::getIOAPICaddr(ioapic_id)+0x3FF);
    Paging::vmap((phys_addr_t)MP::getIOAPICaddr(ioapic_id),
        (virt_addr_t)MP::getIOAPICaddr(ioapic_id), 1, 
        PAGING_PAGE_SUPERVISOR);

    // Build the map of the IOAPIC irq
    for (MP::configuration_table::iterator it 
            = MP::configTablePtr()->begin(); 
        it != MP::configTablePtr()->end();
        it ++)
    {
        if (it.type() == MP::IOINTERRUPT
            && it.as_iointerrupt().dest_id == ioapic_id
            )
        {
            uint8_t apic_irq = it.as_iointerrupt().dest_irq;
        
            switch (MP::busType(it.as_iointerrupt().source_id))
            {
                case MP_CFG_BUS_ISA:
                    ioMap[apic_irq].bus_id = Interrupts::Manager::BUS_ISA;
                    ioMap[apic_irq].bus_irq = it.as_iointerrupt().source_irq;
                    break;
                    
                case MP_CFG_BUS_PCI:
                    ioMap[apic_irq].bus_id = Interrupts::Manager::BUS_PCI;
                    ioMap[apic_irq].bus_irq = it.as_iointerrupt().source_irq;
                    break;
            }
        }
    }
    _initLAPIC();
}

void 
APICManager::shutdown()
{
}

void 
APICManager::enable(bus_type_t bus, uint8_t irq)
{  
    int8_t ioapicIrq = _getIrqFromBus(bus, irq);
    
    if (ioapicIrq == -1)
        return;
        
    ioapic.enableIRQ(ioapicIrq);
}

void 
APICManager::disable(bus_type_t bus, uint8_t irq)
{
    int8_t ioapicIrq = _getIrqFromBus(bus, irq);
    
    if (ioapicIrq == -1)
        return;
        
    ioapic.disableIRQ(ioapicIrq);
}


void 
APICManager::map(bus_type_t bus, uint8_t irq, uint8_t vector)
{
    int8_t ioapicIrq = _getIrqFromBus(bus, irq);
    
    if (ioapicIrq == -1)
        return;
        
    ioapic.setHandler(vector, ioapicIrq, false, false, 0, false);
}

void 
APICManager::eoi(uint64_t vector)
{
    _writeLAPIC(APIC_REG_EOI, 0);
}

int8_t 
APICManager::_getIrqFromBus(bus_type_t bus_id, uint8_t bus_irq)
{
    for (unsigned i = 0; i < 24; i++)
    {
        if (ioMap[i].bus_id == bus_id && ioMap[i].bus_irq == bus_irq)
            return i;
    }

    printf("Can't find irq for bus:%d/irq:%d\n", bus_id, bus_irq);    
    return -1;
}

void
APICManager::_initLAPIC()
{
    uint32_t hi, lo;
    
    __read_msr(IA32_APIC_BASE, &lo, &hi);
    
    // Extract the apic base address
    APIC_BASE_ADDRESS = (uint32_t*)(phys_addr_t)((lo & 0xFFFFF000) | ((uint64_t)hi << 32));
        
    PhysicalMemoryAllocator::mark_reserved((phys_addr_t)APIC_BASE_ADDRESS, 
        (phys_addr_t)((uintptr_t)APIC_BASE_ADDRESS+PAGING_PAGE_SIZE));
    Paging::vmap((phys_addr_t)APIC_BASE_ADDRESS, (virt_addr_t)APIC_BASE_ADDRESS, 1, 0);
}

uint32_t 
APICManager::_readLAPIC(uint16_t reg)
{
    uint32_t r;
    
    // 32-bit read on a 128-bit aligned boundary
    r = *(uint32_t *)((uintptr_t)APIC_BASE_ADDRESS + reg);
    
    return r;
}


void 
APICManager::_writeLAPIC(uint16_t reg, uint32_t value)
{
    *(uint32_t *)((uintptr_t)APIC_BASE_ADDRESS + reg) = value;
}
