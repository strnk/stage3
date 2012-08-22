#ifndef _HW_PM_ALLOC_H_
#define _HW_PM_ALLOC_H_

#include <kernel/types.h>

#define PM_PAGE_SIZE    4096
#define PM_PAGE_BITS    12

#define PM_PAGE_ALIGN_INF(phys_addr) (((uint64_t)(phys_addr) >> PM_PAGE_BITS) << PM_PAGE_BITS)
#define PM_PAGE_ALIGN_SUP(phys_addr) (PM_PAGE_ALIGN_INF(phys_addr) + PM_PAGE_SIZE)

#define PM_DATA_BEGIN_ADDR    PM_PAGE_ALIGN_SUP(& __e_kernel)

namespace Stage3
{
namespace PhysicalMemoryAllocator
{

    struct page_descr
    {
        phys_addr_t addr;
        uint8_t ref_count;
        
        struct page_descr *prev, *next;
    };    

    int
    init(uint64_t ram_size);

    int
    mark_reserved(phys_addr_t lower, phys_addr_t upper);

    phys_addr_t
    getpage();

    int
    free(phys_addr_t page);

    void
    state_dump();

}
}
#endif // _HW_PM_ALLOC_H_
