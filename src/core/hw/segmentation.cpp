#include <kernel/hw/segmentation.hpp>
#include <kernel/hw/segments.h>
    
using namespace Stage3::GDT;

static struct gdt64_register GDTR __align(0x08);
static struct gdt_entry GDT64[] __align(0x08) = 
    {
        gdt_entry(0, 0),
        gdt_entry(0, GDTE_CODE_SEGMENT),
        gdt_entry(0, GDTE_DATA_SEGMENT)
    };
    
Stage3::GDT::gdt_entry::gdt_entry(uint8_t privilege, bool exec)
    : size_lo(0), base_address_lo(0), base_address_mid(0), accessed(0), rw(0), 
    dc(GDTE_DC_CODE_DPL_STRICT), execute(exec), type(GDTE_TYPE_CODE_DATA), 
    dpl(privilege), present(GDTE_PRESENT), size_hi(0), avl(0), longmode(1), 
    op_size(0), granularity(0), base_address_hi(0)
{
}

void
Stage3::GDT::init(void)
{
    GDTR.base_address = (uint64_t)GDT64;
    GDTR.size = sizeof(GDT64) - 1;
    
    // Load the GDT
    asm volatile("lgdt %0"  : : "m"(GDTR) : "memory");
}
