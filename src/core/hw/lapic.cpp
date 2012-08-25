#define _NO_EXTERN_APIC_ADDR
#include <kernel/hw/lapic.hpp>
#include <kernel/hw/paging.hpp>
#include <kernel/hw/pm_alloc.hpp>
#include <kernel/hw/mp.hpp>
#include <kernel/hw/msr.h>


using namespace Stage3;
using namespace Stage3::Interrupts;

APICManager::APICManager()
{
}

uint8_t APICManager::getType()
{
    return 1;
}

void APICManager::init()
{
    ioapic_id = MP::getFirstIOAPIC();
    ioapic = Interrupts::IOAPIC(MP::getIOAPICaddr(ioapic_id));

    PhysicalMemoryAllocator::mark_reserved(
        MP::getIOAPICaddr(ioapic_id), MP::getIOAPICaddr(ioapic_id)+0x3FF);
    Paging::vmap((phys_addr_t)MP::getIOAPICaddr(ioapic_id),
        (virt_addr_t)MP::getIOAPICaddr(ioapic_id), 1, 
        PAGING_PAGE_SUPERVISOR);

    for (MP::configuration_table::iterator it 
            = MP::configTablePtr()->begin(); 
        it != MP::configTablePtr()->end();
        it ++)
    {
        if (it.type() == MP::IOINTERRUPT
            && it.as_iointerrupt().dest_id == ioapic_id
            )
        {
            uint8_t irq = it.as_iointerrupt().source_irq;
            uint8_t apic_irq = it.as_iointerrupt().dest_irq;
        
            switch (MP::busType(it.as_iointerrupt().source_id))
            {
                case MP_CFG_BUS_ISA:
                {
                    ioapic.map(INTERRUPTS_MASKABLE_BASE + irq, 
                        apic_irq, false, false, 0, false);
                }
            }
        }
    }
}

void APICManager::shutdown()
{
}

void APICManager::enable(uint8_t irq)
{
}

void APICManager::disable(uint8_t irq)
{
}


void APICManager::map(uint8_t irq, uint8_t vector)
{
}

void APICManager::eoi(uint64_t vector)
{
}


uint32_t* APIC_BASE_ADDRESS;

void 
init_apic(phys_addr_t relocation)
{
    uint32_t hi, lo;
   
    __read_msr(IA32_APIC_BASE, &lo, &hi);
    
    if (relocation != 0)
    {
        lo &= 0x00000FFF;
        lo |= (relocation & 0xFFFFF000);
        hi = (relocation & 0xFF00000000) >> 32;
        
        __write_msr(IA32_APIC_BASE, lo, hi);
        APIC_BASE_ADDRESS = (uint32_t*)relocation;   
    }
    else
        // Extract the apic base address
        APIC_BASE_ADDRESS = (uint32_t*)(phys_addr_t)((lo & 0xFFFFF000) | ((uint64_t)hi << 32));
        
    PhysicalMemoryAllocator::mark_reserved((phys_addr_t)APIC_BASE_ADDRESS, 
        (phys_addr_t)((uintptr_t)APIC_BASE_ADDRESS+PAGING_PAGE_SIZE));
    Paging::vmap((phys_addr_t)APIC_BASE_ADDRESS, (virt_addr_t)APIC_BASE_ADDRESS, 1, 0);
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
