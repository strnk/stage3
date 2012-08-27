#ifndef _HW_KHEAP_H_
#define _HW_KHEAP_H_

#include <kernel/types.h>
#include <stddef.h>
#include <stdlib.h>

#define KHEAP_BASE                  0xfffffffd00000000

#define KHEAP_MINIMUM_BIN           64
#define KHEAP_BIN_MASK              0xFFFFFFFFFFFFFFC0

#define KHEAP_MAGIC_FREE            0x2045204520522046
#define KHEAP_MAGIC_USED            0x2044204520592055

namespace Stage3 {
namespace HeapAllocator {

    typedef struct _alloc_header
    {
        uintptr_t p;
        uint64_t size;
        uint64_t magic;
        uintptr_t reserved;
        
        struct _alloc_header* next() { 
            return (struct _alloc_header*)((uintptr_t)this 
                + sizeof(struct _alloc_header) + size); }
        struct _alloc_header* prev() { 
            return (struct _alloc_header*)p; }
        void* data() { return (void*)((uintptr_t)this 
            + sizeof(struct _alloc_header)); }
        bool isFree() { return magic == KHEAP_MAGIC_FREE; }
        bool isUsed() { return magic == KHEAP_MAGIC_USED; }
    } alloc_header_t;

    void
    init(uint32_t initial_size);
    
    void
    state_dump();
    
    void*
    malloc(size_t size);
    
    void*
    calloc(size_t num, size_t size);
    
    void*
    realloc(void* ptr, size_t size);

    void
    free(void* ptr);
    
}
}

using Stage3::HeapAllocator::malloc;
using Stage3::HeapAllocator::calloc;
using Stage3::HeapAllocator::realloc;
using Stage3::HeapAllocator::free;
    
void *
operator new(size_t size);
 
void *
operator new[](size_t size);
 
void 
operator delete(void *p);
 
void 
operator delete[](void *p);

/** Placement operators */
inline void *operator new(size_t, void *p)     throw() { return p; }
inline void *operator new[](size_t, void *p)   throw() { return p; }
inline void  operator delete  (void *, void *) throw() { };
inline void  operator delete[](void *, void *) throw() { };

#endif // _HW_KHEAP_H_
