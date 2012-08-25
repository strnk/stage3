#include <kernel/hw/pm_alloc.hpp>
#include <kernel/die.hpp>
#include <stdio.h>

extern unsigned char __e_kernel;
extern unsigned char __e_kernel_phys;
extern unsigned char __b_kernel_phys;

using namespace Stage3::PhysicalMemoryAllocator;

namespace Stage3 {
namespace PhysicalMemoryAllocator {
    circular_list pm_free_pages;
    circular_list pm_used_pages;

    uint32_t pm_total_pages_count;
    uint32_t pm_used_pages_count;
}
}


void
Stage3::PhysicalMemoryAllocator::state_dump()
{
    printf("PM allocator: using %d/%d pages\n", 
        pm_used_pages_count,
        pm_total_pages_count);
}

int
Stage3::PhysicalMemoryAllocator::init(uint64_t ram_size)
{    
    uint64_t page_count = ram_size >> PM_PAGE_BITS;
    uint64_t kernel_begin = PM_PAGE_ALIGN_INF(& __b_kernel_phys);
    uint64_t kernel_end = PM_PAGE_ALIGN_SUP(
        PM_DATA_BEGIN_ADDR_PHYS + page_count * sizeof(struct page_descr));
    unsigned int i;
    struct page_descr* page;
    
    pm_free_pages = circular_list();
    pm_used_pages = circular_list();
    
    /** Page 0 is reserved */
    page = ((struct page_descr*) PM_DATA_BEGIN_ADDR);
    page->prev = NULL;
    page->next = NULL;
    page->addr = 0;
    page->ref_count = 255;
    
    pm_used_pages.insert(page);
    
    /** Mark pages 1-N as free */
    for (i = 1, page = ((struct page_descr*)PM_DATA_BEGIN_ADDR)+1; 
        i < page_count; 
        i++, page++)
    {
        page->ref_count = 255;
        page->addr = i << PM_PAGE_BITS;  
        
        if (i > 3)
            pm_free_pages.__fast_insert_end(page);
        else      
            pm_free_pages.insert(page);
    }
          
    pm_total_pages_count = page_count;
    pm_used_pages_count = 1;
    
    mark_reserved(kernel_begin, kernel_end);
    
    return 0;
}

int
Stage3::PhysicalMemoryAllocator::mark_reserved(phys_addr_t lower, phys_addr_t upper)
{
    phys_addr_t i;
    struct page_descr* page, *next;
    
    lower = PM_PAGE_ALIGN_INF(lower);
    upper = PM_PAGE_ALIGN_SUP(upper);
    
    pm_used_pages_count += pm_free_pages.move(lower, upper, pm_used_pages);
    
    return 0;
}

phys_addr_t
Stage3::PhysicalMemoryAllocator::getpage()
{
    struct page_descr* rpage = pm_free_pages.head();
    
    if (rpage == NULL)
    { puts("Out of memory !"); for (;;) asm volatile("hlt" ::: "memory"); }
//        die("Oops, out of memory !");
        
    pm_free_pages.remove(rpage);
    pm_used_pages.insert(rpage);
    
    pm_used_pages_count++;
    rpage->ref_count = 1;
    
    return rpage->addr;
}

int
Stage3::PhysicalMemoryAllocator::free(phys_addr_t addr)
{    
    int i;
    addr = PM_PAGE_ALIGN_INF(addr);
    struct page_descr* page = pm_used_pages.find(addr);
    
    if (page == NULL)
    {
        printf("%lx free fail: not in used pages\n", page->addr);
        return -1;
    }
    else
    {
        page->ref_count--;
        
        if (page->ref_count == 0)
        {
            pm_used_pages.remove(page);
            pm_free_pages.insert(page);
            
            pm_used_pages_count--;
        }
        
        return page->ref_count;
    }
}

/** Circular list helpers */
void
circular_list::insert(struct page_descr *elmt)
{
    if (begin == NULL)
    {
        begin = elmt;
        begin->next = begin;
        begin->prev = begin;
    }
    else if (begin->next == begin)
    {
        begin->next = elmt;
        begin->prev = elmt;
        elmt->prev = begin;
        elmt->next = begin;
        
        if (elmt->addr < begin->addr)
            begin = elmt;
    }
    else
    {
        struct page_descr *prev, *current;
        
        for (prev = begin->prev, current = begin;
            current->addr < elmt->addr && current != begin->prev;
            prev = current, current = current->next);
            
        prev->next = elmt;
        elmt->prev = prev;
        elmt->next = current;
        current->prev = elmt;
        
        if (elmt->addr < begin->addr)
            begin = elmt;
    }
    
    count++;
}


void
circular_list::__fast_insert_end(struct page_descr *elmt)
{
    begin->prev->next = elmt;
    elmt->prev = begin->prev;
    elmt->next = begin;
    begin->prev = elmt;
    count++;
}
        
struct page_descr*
circular_list::find(phys_addr_t elmt)
{
    int i;
    struct page_descr* it;
    
    for (i = 0, it = begin; (!i) || (it != begin); i++, it = it->next)
    {
        if (it->addr == elmt)
            return it;
    }
    
    return NULL;
}
        
void
circular_list::remove(struct page_descr *elmt)
{
    if (elmt == begin && elmt->next == begin)
    {
        begin = NULL;
    }
    else
    {
        elmt->prev->next = elmt->next;
        elmt->next->prev = elmt->prev;
        
        if (elmt == begin)
            begin = elmt->next;
    }
    
    elmt->next = NULL;
    elmt->prev = NULL;
    count--;
}


uint32_t
circular_list::move(phys_addr_t addr_begin, phys_addr_t addr_end, 
    circular_list& dest)
{
    if (begin == NULL)
        return 0;
    
    unsigned i, total = count, moved = 0;    
    struct page_descr* page, *next;
   
    for (i = 0, page = begin;
        (i < total && page->addr <= addr_end);
        i++, page = next)
    {
        next = page->next;
        
        if (page->addr >= addr_begin && page->addr <= addr_end)
        {
            remove(page);
            dest.insert(page);
            moved++;
        }
    }
    
    return moved;
}

struct page_descr*
circular_list::head()
{
    return begin;
}
