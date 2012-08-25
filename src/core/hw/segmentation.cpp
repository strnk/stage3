#include <kernel/hw/segmentation.hpp>
#include <kernel/hw/segments.h>
    
#include <cstdio>

using namespace Stage3::GDT;

struct gdt64_register GDTR __align(0x08);
struct gdt_entry GDT64[3] __align(0x08) = 
    {
        gdt_entry(),
        gdt_entry(0, GDTE_CODE_SEGMENT),
        gdt_entry(0, GDTE_DATA_SEGMENT)
    };

void
Stage3::GDT::init(void)
{
    GDTR.base_address = (uint64_t)GDT64;
    GDTR.size = sizeof(GDT64) - 1;
    
    // Load the GDT
    asm volatile("lgdt %0"  : : "m"(GDTR) : "memory");
}

void
Stage3::GDT::dump(void)
{

    int i;
    
    for (i = 0; i < 3; i++)
    {
        printf("GDT#%d: %lx\n", i, *(uint64_t*)&(GDT64[i]));
    }
}
