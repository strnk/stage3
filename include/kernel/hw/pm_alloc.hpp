#ifndef _HW_PM_ALLOC_H_
#define _HW_PM_ALLOC_H_

#include <kernel/types.h>
#include <stdlib.h>

#define PM_PAGE_SIZE    4096
#define PM_PAGE_BITS    12

#define PM_PAGE_ALIGN_INF(phys_addr) (((uint64_t)(phys_addr) >> PM_PAGE_BITS) << PM_PAGE_BITS)
#define PM_PAGE_ALIGN_SUP(phys_addr) (PM_PAGE_ALIGN_INF(phys_addr) + PM_PAGE_SIZE)

#define PM_DATA_BEGIN_ADDR      PM_PAGE_ALIGN_SUP(& __e_kernel)
#define PM_DATA_BEGIN_ADDR_PHYS PM_PAGE_ALIGN_SUP(& __e_kernel_phys)

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

    class circular_list
    {
        struct page_descr *begin;
        uint32_t count;
        
        public:
        circular_list()
            : begin(NULL), count(0) { }
            
        void
        insert(struct page_descr *elmt);
        
        void
        __fast_insert_end(struct page_descr *elmt);
        
        struct page_descr*
        find(phys_addr_t elmt);
        
        void
        remove(struct page_descr *elmt);
        
        uint32_t
        move(phys_addr_t addr_begin, phys_addr_t addr_end, circular_list& dest);
        
        struct page_descr* 
        head();
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
