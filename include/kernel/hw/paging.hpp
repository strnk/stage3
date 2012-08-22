#ifndef _HW_PAGING_H_
#define _HW_PAGING_H_

#include <kernel/types.h>

#define PAGING_PAGE_SIZE            4096

#define PAGING_PAGE_PRESENT         0x01
#define PAGING_PAGE_DISABLED        0x00

#define PAGING_PAGE_RW              0x01
#define PAGING_PAGE_READ_ONLY       0x00

#define PAGING_PAGE_USER            0x01
#define PAGING_PAGE_SUPERVISOR      0x00

#define PAGING_PAGE_WRITE_THROUGH   0x01

#define PAGING_PAGE_CACHE_DISABLED  0x01

#define PAGING_PAGE_ACCESSED        0x01

namespace Stage3 {
namespace Paging {

    struct pml4t_entry;
    struct pdpt_entry;
    struct pd_entry;
    struct pt_entry;
    
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
        
        pml4t_entry(phys_addr_t base, bool present, bool rw, bool user, 
            bool pwt, bool pcd, bool nx)
            : present(present), rw(rw), user(user), pwt(pwt), pcd(pcd),
            accessed(0), reserved1(0), __avail1(0), 
            base_lo(((base) & 0x000000000000F000) >> 12),
            base_mid(((base) & 0x00000000FFFF0000) >> 16),
            base_hi(((base) & 0x000000FF00000000) >> 32),
            reserved2(0), __avail2(0), noexec(nx) { };  
            
        pml4t_entry()
            : present(0), rw(0), user(0), pwt(0), pcd(0),
            accessed(0), reserved1(0), __avail1(0), 
            base_lo(0), base_mid(0), base_hi(0),
            reserved2(0), __avail2(0), noexec(0) { };  
            
        struct pdpt_entry* ptr();
        bool isPresent() { return ((present & PAGING_PAGE_PRESENT) != 0); }
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
        
        pdpt_entry(phys_addr_t base, bool present, bool rw, bool user, 
            bool pwt, bool pcd, bool nx)
            : present(present), rw(rw), user(user), pwt(pwt), pcd(pcd),
            accessed(0), reserved1(0), __avail1(0), 
            base_lo(((base) & 0x000000000000F000) >> 12),
            base_mid(((base) & 0x00000000FFFF0000) >> 16),
            base_hi(((base) & 0x000000FF00000000) >> 32),
            reserved2(0), __avail2(0), noexec(nx) { };  
            
        pdpt_entry()
            : present(0), rw(0), user(0), pwt(0), pcd(0),
            accessed(0), reserved1(0), __avail1(0), 
            base_lo(0), base_mid(0), base_hi(0),
            reserved2(0), __avail2(0), noexec(0) { }; 
            
        static struct pdpt_entry* init();
        struct pd_entry* ptr();
        bool isPresent() { return ((present & PAGING_PAGE_PRESENT) != 0); }
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

        
        pd_entry(phys_addr_t base, bool present, bool rw, 
            bool user, bool pwt, bool pcd, bool size, bool nx)
            : present(present), rw(rw), user(user), pwt(pwt), pcd(pcd),
            accessed(0), reserved1(0), size(size), reserved2(0), 
            __avail1(0), base_lo(((base) & 0x000000000000F000) >> 12),
            base_mid(((base) & 0x00000000FFFF0000) >> 16),
            base_hi(((base) & 0x000000FF00000000) >> 32),
            reserved3(0), __avail2(0), noexec(nx) { };
            
        pd_entry()
            : present(0), rw(0), user(0), pwt(0), pcd(0),
            accessed(0), reserved1(0), size(0), reserved2(0), 
            __avail1(0), base_lo(0), base_mid(0), base_hi(0),
            reserved3(0), __avail2(0), noexec(0) { };
            
        static struct pd_entry* init();
        struct pt_entry* ptr();
        bool isPresent() { return ((present & PAGING_PAGE_PRESENT) != 0); }
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
        
        
        pt_entry(phys_addr_t base, bool pat, bool present, bool rw, 
            bool user, bool pwt, bool pcd, bool global, bool nx)
            : present(present), rw(rw), user(user), pwt(pwt), pcd(pcd),
            accessed(0), dirty(0), pat(pat), global(global), __avail1(0),
            base_lo(((base) & 0x000000000000F000) >> 12),
            base_mid(((base) & 0x00000000FFFF0000) >> 16),
            base_hi(((base) & 0x000000FF00000000) >> 32),
            reserved1(0), __avail2(0), noexec(nx) { };
            
        pt_entry()
            : present(0), rw(0), user(0), pwt(0), pcd(0),
            accessed(0), dirty(0), pat(0), global(0), __avail1(0),
            base_lo(0),
            base_mid(6),
            base_hi(0),
            reserved1(0), __avail2(0), noexec(0) { };
            
        static struct pt_entry* init();
        phys_addr_t ptr();
        bool isPresent() { return ((present & PAGING_PAGE_PRESENT) != 0); }
    } __pack;


    // Linear address
    struct linear_addr
    {
        unsigned int offset          : 12;
        unsigned int table           : 9;
        unsigned int directory       : 9;
        unsigned int directory_ptr   : 9;
        unsigned int pml4            : 9;
        
        linear_addr(virt_addr_t vaddr);
    } __pack;


    // Table offset are 9 or 12 bits max.
    typedef uint16_t table_offset_t;
        
    #define PML4Toff(linear)    ((linear).pml4 & 0x1FF)
    #define PDPToff(linear)     ((linear).directory_ptr & 0x1FF)
    #define PDoff(linear)       ((linear).directory & 0x1FF)
    #define PToff(linear)       ((linear).table & 0x1FF)

    // Initialization helpers
    void 
    init();

    // VMAP management
    int
    vmap(phys_addr_t paddr, virt_addr_t vaddr, uint32_t npage, bool user);
        
    int
    vunmap(virt_addr_t vaddr, uint32_t npage);

}
}
#endif // _HW_PAGING_H_
