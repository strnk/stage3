#ifndef _HW_PM_ALLOC_H_
#define _HW_PM_ALLOC_H_

#include <kernel/types.h>

#define PM_PAGE_SIZE    4096
#define PM_PAGE_BITS    12

#define PM_PAGE_ALIGN_INF(phys_addr) (((uint64_t)(phys_addr) >> PM_PAGE_BITS) << PM_PAGE_BITS)
#define PM_PAGE_ALIGN_SUP(phys_addr) (PM_PAGE_ALIGN_INF(phys_addr) + PM_PAGE_SIZE)

#define PM_DATA_BEGIN_ADDR    PM_PAGE_ALIGN_SUP(& __e_kernel)


struct pm_page_descr
{
    phys_addr_t addr;
    uint8_t ref_count;
    
    struct pm_page_descr *prev, *next;
};

void
pm_state_dump();

int
pm_alloc_init(uint64_t ram_size);

int
pm_alloc_mark_reserved(phys_addr_t lower, phys_addr_t upper);

phys_addr_t
pm_alloc_getpage();

int
pm_alloc_free(phys_addr_t page);


#endif // _HW_PM_ALLOC_H_
