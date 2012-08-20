#ifndef _HW_PAGING_H_
#define _HW_PAGING_H_

#include <kernel/types.h>

#define PAGING_PAGE_SIZE            4096

#define PAGING_BASE                 0xfffffffc00000000
#define VPAGE_PAGING_PT             PAGING_BASE
#define VPAGE_PML4T                 1
#define VPAGE_PDPT                  2
#define VPAGE_PD                    3
#define VPAGE_PT                    4
#define VPAGE_PDPT2                 5
#define VPAGE_PD2                   6
#define VPAGE_PT2                   7

#define __VLOAD_PAGE(idx)           ((void*)PAGING_BASE + idx*PAGING_PAGE_SIZE)

#define PAGING_PAGE_PRESENT         0x01
#define PAGING_PAGE_DISABLED        0x00

#define PAGING_PAGE_RW              0x01
#define PAGING_PAGE_READ_ONLY       0x00

#define PAGING_PAGE_USER            0x01
#define PAGING_PAGE_SUPERVISOR      0x00

#define PAGING_PAGE_WRITE_THROUGH   0x01

#define PAGING_PAGE_CACHE_DISABLED  0x01

#define PAGING_PAGE_ACCESSED        0x01

// Page-map level 4 table
struct pml4t_entry
{
    int         present     : 1;        // 0 : page disabled, 1: page present
    int         rw          : 1;        // 0 : read-only, 1 : read-write
    int         user        : 1;        // 0 : supervisor, 1 : user
    int         pwt         : 1;        // 0 : write-through disabled, 1: enabled
    int         pcd         : 1;        // 0 : cache enable, 1 : cache disabled
    int         accessed    : 1;
    int         reserved1   : 3;
    int         __avail1    : 3;
    
    int         base_lo     : 4;        // base[12..15]
    uint16_t   base_mid;               // base[16..31]
    uint8_t    base_hi;                // base[32..39]
    
    int         reserved2   : 12;
    int         __avail2    : 11;
    int         noexec      : 1;
} __pack;


// Page-directory pointer table
struct pdpt_entry
{
    int         present     : 1;        // 0 : page disabled, 1: page present
    int         rw          : 1;        // 0 : read-only, 1 : read-write
    int         user        : 1;        // 0 : supervisor, 1 : user
    int         pwt         : 1;        // 0 : write-through disabled, 1: enabled
    int         pcd         : 1;        // 0 : cache enable, 1 : cache disabled
    int         accessed    : 1;
    int         reserved1   : 3;
    int         __avail1    : 3;
    
    int         base_lo     : 4;        // base[12..15]
    uint16_t   base_mid;               // base[16..31]
    uint8_t    base_hi;                // base[32..39]
    
    int         reserved2   : 12;
    int         __avail2    : 11;
    int         noexec      : 1;
} __pack;


// Page-directory
struct pd_entry
{
    int         present     : 1;        // 0 : page disabled, 1: page present
    int         rw          : 1;        // 0 : read-only, 1 : read-write
    int         user        : 1;        // 0 : supervisor, 1 : user
    int         pwt         : 1;        // 0 : write-through disabled, 1: enabled
    int         pcd         : 1;        // 0 : cache enable, 1 : cache disabled
    int         accessed    : 1;
    int         reserved1   : 1;
    int         size        : 1;        // 0 : 4KB, 1: 4MB
    int         reserved2   : 1;
    int         __avail1    : 3;
    
    int         base_lo     : 4;        // base[12..15]
    uint16_t   base_mid;               // base[16..31]
    uint8_t    base_hi;                // base[32..39]
    
    int         reserved3   : 12;
    int         __avail2    : 11;
    int         noexec      : 1;
} __pack;



// Page table
struct pt_entry
{
    int         present     : 1;        // 0 : page disabled, 1: page present
    int         rw          : 1;        // 0 : read-only, 1 : read-write
    int         user        : 1;        // 0 : supervisor, 1 : user
    int         pwt         : 1;        // 0 : write-through disabled, 1: enabled
    int         pcd         : 1;        // 0 : cache enable, 1 : cache disabled
    int         accessed    : 1;
    int         dirty       : 1;
    int         pat         : 1;
    int         global      : 1;
    int         __avail1    : 3;
    
    int         base_lo     : 4;        // base[12..15]
    uint16_t   base_mid;               // base[16..31]
    uint8_t    base_hi;                // base[32..39]
    
    int         reserved1   : 12;
    int         __avail2    : 11;
    int         noexec      : 1;
} __pack;


#define PAGE_PTR(te) ( \
                       (((phys_addr_t)(te)->base_lo & 0xFF) << 12) \
                     | (((phys_addr_t)(te)->base_mid) << 16)       \
                     | (((phys_addr_t)(te)->base_hi) << 32))
                     
#define PDPT_ADDR(te) ((struct pdpt_entry*)PAGE_PTR(te))
#define PD_ADDR(te) ((struct pd_entry*)PAGE_PTR(te))
#define PT_ADDR(te) ((struct pt_entry*)PAGE_PTR(te))

// Virtual address
struct linear_addr
{
    unsigned int offset          : 12;
    unsigned int table           : 9;
    unsigned int directory       : 9;
    unsigned int directory_ptr   : 9;
    unsigned int pml4            : 9;
} __pack;


// Table offset are 9 or 12 bits max.
typedef uint16_t table_offset_t;

#define VADDR_TO_LINEAR(vaddr)                                  \
    ((struct linear_addr) {                                     \
        .offset         = ((vaddr) & 0x0000000000000FFF),       \
        .table          = ((vaddr) & 0x00000000001FF000) >> 12, \
        .directory      = ((vaddr) & 0x000000003FE00000) >> 21, \
        .directory_ptr  = ((vaddr) & 0x0000007FC0000000) >> 30, \
        .pml4           = ((vaddr) & 0x0000FF8000000000) >> 39  \
    })
    
#define PML4Toff(linear)    ((linear).pml4 & 0x1FF)
#define PDPToff(linear)     ((linear).directory_ptr & 0x1FF)
#define PDoff(linear)       ((linear).directory & 0x1FF)
#define PToff(linear)       ((linear).table & 0x1FF)

// Initialization helpers
void 
paging_init();

struct pdpt_entry*
pdpt_init();

struct pd_entry*
pd_init();

struct pt_entry*
pt_init();

// VMAP management

int
paging_vmap(phys_addr_t paddr, virt_addr_t vaddr, uint32_t npage, 
    uint8_t user);
    
int
paging_vunmap(virt_addr_t vaddr, uint32_t npage);
#endif // _HW_PAGING_H_
