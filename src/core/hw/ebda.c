#include <kernel/hw/ebda.h>
#include <stdio.h>

const uint16_t* bda = (const uint16_t*)0x400;
const uint32_t* ebda;

/** Private helpers */
phys_addr_t
_search_u32_id(uint32_t id, phys_addr_t b, size_t l);

phys_addr_t
_search_u64_id(uint64_t id, phys_addr_t b, size_t l);


void
ebda_init()
{
    ebda = (const uint32_t*)((uintptr_t)bda[BDA_EBDA_ADDR] << 4);
}

phys_addr_t
ebda_search_u32_id(uint32_t id)
{
    phys_addr_t addr;
    
    addr = _search_u32_id(id, (phys_addr_t)ebda, 0x400);
    if (addr != (phys_addr_t)0) return addr;
    
    addr = _search_u32_id(id, (phys_addr_t)0x9FC00, 0x400);
    if (addr != (phys_addr_t)0) return addr;
    
    addr = _search_u32_id(id, (phys_addr_t)0xF0000, 0x10000);
    return addr;
}


phys_addr_t
ebda_search_u64_id(uint64_t id)
{
    phys_addr_t addr;
    
    addr = _search_u64_id(id, (phys_addr_t)ebda, 0x400);
    if (addr != (phys_addr_t)0) return addr;
    
    addr = _search_u64_id(id, (phys_addr_t)0x9C00, 0x400);
    if (addr != (phys_addr_t)0) return addr;
    
    addr = _search_u64_id(id, (phys_addr_t)0xF0000, 0x10000);
    return addr;
}

phys_addr_t
_search_u32_id(uint32_t id, phys_addr_t b, size_t l)
{    
    const uint32_t* ptr;
    uintptr_t end = b+l;
    
    for (ptr = (uint32_t*)b; (uintptr_t)ptr < end; ptr = (void*)ptr + 16)
        if (*ptr == id)
            return (phys_addr_t)ptr;   
               
    return (phys_addr_t)0;
}

phys_addr_t
_search_u64_id(uint64_t id, phys_addr_t b, size_t l)
{    
    const uint64_t* ptr;
    uintptr_t end = b+l;
    
    for (ptr = (uint64_t*)b; (uintptr_t)ptr < end; ptr = (void*)ptr + 16)
        if (*ptr == id)
            return (phys_addr_t)ptr;   
               
    return (phys_addr_t)0;
}
