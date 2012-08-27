#include <kernel/hw/kheap.hpp>
#include <kernel/hw/paging.hpp>
#include <kernel/hw/pm_alloc.hpp>

#include <cstdio>

using namespace Stage3;
using namespace Stage3::HeapAllocator;

static uintptr_t heap_end = KHEAP_BASE;

void _allocHeapPage();
void _splitChunk(alloc_header_t *it, size_t size);

void
Stage3::HeapAllocator::init(uint32_t initial_size)
{
    while (heap_end - KHEAP_BASE < initial_size)
        _allocHeapPage();
        
    ((alloc_header_t*)KHEAP_BASE)->size = heap_end - KHEAP_BASE 
        - sizeof(alloc_header_t);
    ((alloc_header_t*)KHEAP_BASE)->magic = KHEAP_MAGIC_FREE;
    ((alloc_header_t*)KHEAP_BASE)->p = NULL;
}

void
Stage3::HeapAllocator::state_dump()
{
    alloc_header_t *it;
    
    for (it = (alloc_header_t*)KHEAP_BASE; (uintptr_t)it < heap_end; 
         it = it->next())
        printf("%lx - %lx: %s %ld bytes\n", (uintptr_t)it+sizeof(alloc_header_t), 
            (uintptr_t)it + sizeof(alloc_header_t) + it->size - 1,
            (it->isFree()?"free":"used"), it->size);
}

void*
Stage3::HeapAllocator::malloc(size_t size)
{
    if (size == 0)
        return NULL;
        
    alloc_header_t *it, *p;
    
    if (size % KHEAP_MINIMUM_BIN)
        size = (size & KHEAP_BIN_MASK) + KHEAP_MINIMUM_BIN;
    
    for (p = it = (alloc_header_t*)KHEAP_BASE; (uintptr_t)it < heap_end; 
        p = it,  it = it->next())
    {                
        if (it->isFree() && it->size >= size)
        {
            // Don't split the chunk if it's too small
            if (it->size - size < KHEAP_MINIMUM_BIN)
                it->magic = KHEAP_MAGIC_USED;
            else
                _splitChunk(it, size);
                
            return it->data();
        }
    }

    // Make the heap growing    
    for (unsigned i = PM_PAGE_ALIGN_SUP(size - p->size);
        i > 0; i -= PM_PAGE_SIZE)
        _allocHeapPage();
        
    p->size = heap_end - (uintptr_t)p - sizeof(alloc_header_t);
    _splitChunk(p, size);
    
    return p->data();
}

void*
Stage3::HeapAllocator::calloc(size_t num, size_t size)
{
    num = size = num;
    return NULL;
}

void*
Stage3::HeapAllocator::realloc(void* ptr, size_t size)
{
    size = size;
    ptr = ptr;
    return NULL;
}

void
Stage3::HeapAllocator::free(void* ptr)
{
    if (ptr == NULL)
        return;
        
    alloc_header_t *it = 
        (alloc_header_t*)((uintptr_t)ptr-sizeof(alloc_header_t));
    
    if (it->isFree())
    {
        printf("HeapAllocator: freeing %lx twice !\n", (uintptr_t)ptr);
        return;
    }
    
    if (!it->isUsed())
    {
        printf("HeapAllocator: freeing %lx but magic=%lx\n", (uintptr_t)ptr, it->magic);
        return;
    }
    
    it->magic = KHEAP_MAGIC_FREE;
    
    alloc_header_t *n = it->next(), *p = it->prev();
    if ((uintptr_t)n < heap_end && n->isFree())
    {
        it->size += n->size + sizeof(alloc_header_t);
        
        if ((uintptr_t)it->next() < heap_end)
            it->next()->p = (uintptr_t)it;
    }
        
    if (p != NULL && p->isFree())
    {
        p->size += it->size + sizeof(alloc_header_t);
        
        if ((uintptr_t)p->next() < heap_end)
            p->next()->p = (uintptr_t)p;
    }
}

/** C++ **/
void *
operator new(size_t size)
{
    return malloc(size);
}
 
void *
operator new[](size_t size)
{
    return malloc(size);
}
 
void 
operator delete(void *p)
{
    free(p);
}
 
void 
operator delete[](void *p)
{
    free(p);
}

/** Helpers */
void _splitChunk(alloc_header_t *it, size_t size)
{
    alloc_header_t* split = (alloc_header_t*)((uintptr_t) it 
        + size + sizeof(alloc_header_t));
        
    split->p = (uintptr_t)it;
    split->magic = KHEAP_MAGIC_FREE;
    split->size = it->size - size - sizeof(alloc_header_t);
  
    if ((uintptr_t)split->next() < heap_end)  
        split->next()->p = (uintptr_t)split;
  
    it->magic = KHEAP_MAGIC_USED;
    it->size = size;
}

void _allocHeapPage()
{
    phys_addr_t newpage = PhysicalMemoryAllocator::getpage();
    Paging::vmap(newpage, (virt_addr_t)heap_end, 1, false);
    
    if (newpage < 0x120000)
        printf("%lx  ", newpage);
    heap_end += PM_PAGE_SIZE;
}
