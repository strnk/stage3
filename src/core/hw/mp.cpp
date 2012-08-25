#include <kernel/hw/pm_alloc.hpp>
#include <kernel/hw/paging.hpp>
#include <kernel/hw/mp.hpp>
extern "C" {
#include <kernel/hw/ebda.h>
}
#include <cstdio>

using namespace Stage3::MP;

configuration_table* mpTableHeader = NULL;
uint32_t mpTableIOAPIC[8] = { 0, };
uint8_t mpTableBuses[8] = { 0, };

void
Stage3::MP::init()
{
    table_floating_pointer* mpTableFP;
    
    mpTableFP = (table_floating_pointer*)ebda_search_u32_id(
        EBDA_ID32('_', 'M', 'P', '_'));
        
    if (mpTableFP == NULL
        || !mpTableFP->isSignatureValid()
        || !mpTableFP->isChecksumValid())
    {
        puts("Oops, unable to find a valid MP table.");
        return;
    }
    
    if (mpTableFP->feature[0] != 0)
    {
        puts("MP default config");
        return;
    }
    else if (mpTableFP->phys_ptr == 0)
    {
        puts("Oops, invalid MP table ptr.");
        return;
    }
    
    mpTableHeader = (configuration_table*)(uintptr_t)mpTableFP->phys_ptr;
    
    // Mapping the table in virtual memory
    PhysicalMemoryAllocator::mark_reserved(
        (phys_addr_t)mpTableFP->phys_ptr, 
        (phys_addr_t)((uintptr_t)mpTableFP->phys_ptr+PAGING_PAGE_SIZE));
        
    Paging::vmap((phys_addr_t)mpTableFP->phys_ptr, 
        (virt_addr_t)mpTableFP->phys_ptr, 1, 0);
        
    if (!mpTableHeader->isSignatureValid())
    {
        puts("Oops, invalid MP cfg table.");
        return;
    }
    
    configuration_table::iterator it;
    
    for (it = mpTableHeader->begin(); it != mpTableHeader->end(); it++)
    {
        switch (it.type())
        {
            case PROCESSOR:
            case IOINTERRUPT:
            case LINTERRUPT:
            default:
            break;           
            
            case BUS:
                if (it.as_bus().isBus("PCI   "))
                    mpTableBuses[it.as_bus().bus_id] = MP_CFG_BUS_PCI;
                else if (it.as_bus().isBus("ISA   "))
                    mpTableBuses[it.as_bus().bus_id] = MP_CFG_BUS_ISA;
            break;
            
            case IOAPIC:
                if (it.as_ioapic().isEnabled())
                    mpTableIOAPIC[it.as_ioapic().ioapic_id] = 
                        it.as_ioapic().ioapic_address;
            break;
        }
    }
}


bool
Stage3::MP::hasIOAPIC()
{
    for (unsigned int i = 0; 
        i < sizeof(mpTableIOAPIC) / sizeof(uint32_t); i++)
    {
        if (mpTableIOAPIC[i] != 0)
            return true;
    }
    
    return false;
}

uint8_t
Stage3::MP::getFirstIOAPIC()
{
    for (unsigned int i = 0; 
        i < sizeof(mpTableIOAPIC) / sizeof(uint32_t); i++)
    {
        if (mpTableIOAPIC[i] != 0)
            return i;
    }
    
    return 0;
}

uint32_t
Stage3::MP::getIOAPICaddr(uint8_t id)
{
    return mpTableIOAPIC[id];
}

uint8_t
Stage3::MP::busType(uint8_t busId)
{
    return mpTableBuses[busId];
}

configuration_table*
Stage3::MP::configTablePtr()
{
    return mpTableHeader;
}
/** Configuration table helpers */

bool
configuration_table::isSignatureValid()
{
    return (signature[0] == 'P'
        && signature[1] == 'C'
        && signature[2] == 'M'
        && signature[3] == 'P');
}

configuration_table::iterator
configuration_table::begin() { 
    return iterator((uintptr_t)this
        + sizeof(struct configuration_table), 
        entry_count, xtable_length);
}

configuration_table::iterator 
configuration_table::end() {
    return iterator(0, 1, 0);
}
       

/** Configuration table iterators */

configuration_table::iterator::iterator() 
    : p(0), entry_count_left(0), xtable_length_left(0) 
{
}

configuration_table::iterator::iterator(uintptr_t ptr, uint16_t entry_count, 
    uint16_t xtable_length)
    : p(ptr), entry_count_left(entry_count-1), 
    xtable_length_left(xtable_length) 
{ 
}
    
configuration_table::iterator::iterator(const iterator& it) 
    : p(it.p), entry_count_left(it.entry_count_left), 
    xtable_length_left(it.xtable_length_left) 
{ 
}

configuration_table::iterator 
configuration_table::iterator::operator++(int) 
{ 
    configuration_table::iterator tmp(*this);
    operator++(); 
    return tmp; 
}

configuration_table::iterator& 
configuration_table::iterator::operator++() 
{ 
    if (entry_count_left)
    {
        if (*(uint8_t*)p == 0)
            p += 20;
        else
            p += 8;
            
        entry_count_left--;
    }
    else if (xtable_length_left > 0)
    {
        xtable_length_left -= *(uint8_t*)(p+1);
        p += *(uint8_t*)(p+1);
    }
    
    return *this; 
}

bool 
configuration_table::iterator::operator==(
    const configuration_table::iterator& rhs) 
{
    return 
        (entry_count_left == rhs.entry_count_left)
        && (xtable_length_left == rhs.xtable_length_left);
}

bool 
configuration_table::iterator::operator!=(
    const configuration_table::iterator& rhs) 
{ 
    return 
        (entry_count_left != rhs.entry_count_left)
        || (xtable_length_left != rhs.xtable_length_left);
}

config_entry_type_t
configuration_table::iterator::type()
{
    return (config_entry_type_t)(*(uint8_t*)p); 
}

config_entry_processor&
configuration_table::iterator::as_processor()
{
    return *(config_entry_processor*)p;
}

config_entry_bus&
configuration_table::iterator::as_bus()
{
    return *(config_entry_bus*)p;
}

config_entry_ioapic&
configuration_table::iterator::as_ioapic()
{
    return *(config_entry_ioapic*)p;
}

config_entry_iointerrupt&
configuration_table::iterator::as_iointerrupt()
{
    return *(config_entry_iointerrupt*)p;
}

config_entry_linterrupt&
configuration_table::iterator::as_linterrupt()
{
    return *(config_entry_linterrupt*)p;
}

/** Floating pointer helpers */

bool
table_floating_pointer::isSignatureValid()
{
    return (signature[0] == '_')
        && (signature[1] == 'M')
        && (signature[2] == 'P')
        && (signature[3] == '_');
}


bool
table_floating_pointer::isChecksumValid()
{
    uint8_t *ptr = (uint8_t*)this;
    uint8_t checksum = 0;
    
    for (ptr = (uint8_t*)this; 
        (uintptr_t)ptr < (uintptr_t)this + (length*16);
        ptr++)
    {
        checksum += *ptr;
    }
    
    return (checksum == 0);
}
