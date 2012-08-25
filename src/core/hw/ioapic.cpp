#include <kernel/hw/ioapic.hpp>
#include <stddef.h>

using namespace Stage3::Interrupts;

IOAPIC Stage3::Interrupts::bspIOAPIC;

IOAPIC::IOAPIC()
    : IOREGSEL(NULL), IOWIN(NULL)
{
}

IOAPIC::IOAPIC(uintptr_t base)
    : IOREGSEL((uint32_t*)base), IOWIN((uint32_t*)(base+0x10))
{
}

void 
IOAPIC::map(uint8_t vector, uint8_t irq, bool level_sensitive,
    bool active_low, uint8_t destination, bool destination_logical)
{
    ioredtbl_t entry(vector, IOREDTBL_DELIV_FIXED, destination_logical,
        active_low, level_sensitive, true, destination);
 
    ioMap[irq] = vector;       
    _writeReg64(IOREDTBL(irq), *(uint64_t*)&entry);
}

uint8_t
IOAPIC::getIRQfromVector(uint8_t vector)
{
    for (unsigned i = 0; i < sizeof(ioMap); i++)
    {
        if (ioMap[i] == vector)
            return i;
    }
    
    return 0xFF;
}

void
IOAPIC::enableIRQ(uint8_t irq)
{
    uint64_t irqEntry = _readReg64(IOREDTBL(irq));
    
    _writeReg64(IOREDTBL(irq), irqEntry & 0xFFFFFFFFFFFEFFFF);
}

void
IOAPIC::disableIRQ(uint8_t irq)
{
    uint64_t irqEntry = _readReg64(IOREDTBL(irq));
    
    _writeReg64(IOREDTBL(irq), irqEntry | 0x0000000000010000);
}

uint32_t
IOAPIC::_readReg32(uint8_t offset)
{
    *IOREGSEL = (uint32_t) offset;
    return *IOWIN;
}

uint64_t
IOAPIC::_readReg64(uint8_t offset)
{
    return ((uint64_t)_readReg32(offset+1) << 32) | _readReg32(offset);
}

void
IOAPIC::_writeReg32(uint8_t offset, uint32_t value)
{
    *IOREGSEL = (uint32_t) offset;
    *IOWIN = value;
}

void
IOAPIC::_writeReg64(uint8_t offset, uint64_t value)
{
    _writeReg32(offset+1, value >> 32);
    _writeReg32(offset, value & 0xFFFFFFFF);
}
