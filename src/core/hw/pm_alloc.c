#include <kernel/hw/pm_alloc.h>
#include <kernel/die.h>
#include <stdio.h>

extern unsigned char __e_kernel;
extern unsigned char __b_kernel;

struct pm_page_descr* pm_free_pages;
struct pm_page_descr* pm_used_pages;

uint32_t pm_total_pages_count;
uint32_t pm_used_pages_count;


void
pm_state_dump()
{
    printf("PM allocator: using %d/%d pages\n", 
        pm_used_pages_count,
        pm_total_pages_count);
}

void
pm_descr_dump(struct pm_page_descr* pmd)
{
    printf("Addr: 0x%lx\n", pmd->addr);
    printf("Ref#: %d\n", pmd->ref_count);
    printf("Next: 0x%p\n", pmd->next);
    printf("Prev: 0x%p\n", pmd->prev);
}

int
pm_alloc_init(uint64_t ram_size)
{    
    uint64_t page_count = ram_size >> PM_PAGE_BITS;
    uint64_t kernel_begin = PM_PAGE_ALIGN_INF(& __b_kernel);
    uint64_t kernel_end = PM_PAGE_ALIGN_SUP(PM_DATA_BEGIN_ADDR + page_count * sizeof(struct pm_page_descr));
    int i;
    struct pm_page_descr* page;
    
    
    /** Mark pages 1-N as free */
    for (i = 1, page = ((struct pm_page_descr*)PM_DATA_BEGIN_ADDR)+1; 
        i < page_count; 
        i++, page++)
    {
        page->prev = page-1;
        page->next = page+1;
        page->addr = i << PM_PAGE_BITS;
        page->ref_count = 0;
    }
    
    pm_free_pages =  ((struct pm_page_descr*)PM_DATA_BEGIN_ADDR)+1;
    pm_free_pages->prev = page-1;
    pm_free_pages->prev->next = pm_free_pages;
    
    /** Mark page 0 (reserved) as used */
    pm_used_pages = (struct pm_page_descr*)PM_DATA_BEGIN_ADDR;
    pm_used_pages->prev = pm_used_pages;
    pm_used_pages->next = pm_used_pages;
    pm_used_pages->addr = 0;
    pm_used_pages->ref_count = 255;
    
    pm_total_pages_count = page_count;
    pm_used_pages_count = 1;
    
    pm_alloc_mark_reserved(kernel_begin, kernel_end);
    
    return 0;
}

int
pm_alloc_mark_reserved(phys_addr_t lower, phys_addr_t upper)
{
    int i;
    struct pm_page_descr* page;
    struct pm_page_descr* prev;
    
    lower = PM_PAGE_ALIGN_INF(lower);
    upper = PM_PAGE_ALIGN_SUP(upper);
    
    for (i = 0, page = pm_free_pages; (!i || (page != pm_free_pages && page->addr <= upper)); i++, page = page->next)
    {  
    
        if (page->addr >= lower && page->addr <= upper)
        {
            prev = page->prev;
            prev->next = page->next;
            page->next->prev = prev;
            
            pm_used_pages->prev->next = page;
            page->prev = pm_used_pages->prev;
            page->next = pm_used_pages;
            pm_used_pages->prev = page;
            page->ref_count = 255;
            page = prev;
            
            pm_used_pages_count++;
        }
    }
    return 0;
}

phys_addr_t
pm_alloc_getpage()
{
    struct pm_page_descr* rpage = pm_free_pages;
    
    if (pm_free_pages == NULL)
        die("Out of memory");
        
    if (rpage->next == rpage)
    {
        pm_free_pages = NULL;
    }
    else
    {
        rpage->prev->next = rpage->next;
        rpage->next->prev = rpage->prev;
        pm_free_pages = rpage->next;
    }
    
    rpage->prev = pm_used_pages->prev;
    rpage->next = pm_used_pages;
    pm_used_pages->prev->next = rpage;
    pm_used_pages->prev = rpage;
    
    pm_used_pages_count++;
    rpage->ref_count = 1;
    return rpage->addr;
}

int
pm_alloc_free(phys_addr_t addr)
{    
    int i;
    struct pm_page_descr* page;
    addr = PM_PAGE_ALIGN_INF(addr);
    
    for (i = 0, page = pm_used_pages; !i || page != pm_used_pages; i++, page = page->next)
    {
        if (page->addr == addr)
        {
            page->ref_count--;
            
            if (page->ref_count == 0)
            {
                if (page->next == page)
                    pm_used_pages = NULL;
                else
                {
                    page->prev->next = page->next;
                    page->next->prev = page->prev;
                    
                    if (pm_used_pages == page)
                        pm_used_pages = page->next;
                }
                
                if (pm_free_pages == NULL)
                    pm_free_pages = page;
                else
                {
                    pm_free_pages->prev->next = page;
                    page->prev = pm_free_pages->prev;
                    
                    pm_free_pages->prev = page;
                    page->next = pm_free_pages;
                    pm_free_pages = page;
                }
                
                pm_used_pages_count--;
            }
            
            return page->ref_count;
        }
    }
    
    return -1;
}
