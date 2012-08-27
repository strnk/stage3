#include <kernel/hw/paging.hpp>
#include <kernel/hw/pm_alloc.hpp>
#include <cstdio>

#define VPAGE_PAGING_PT             PAGING_BASE
#define VPAGE_PML4T                 1
#define VPAGE_PDPT                  2
#define VPAGE_PD                    3
#define VPAGE_PT                    4
#define VPAGE_PDPT2                 5
#define VPAGE_PD2                   6
#define VPAGE_PT2                   7

#define __VLOAD_PAGE(idx)           ((uintptr_t)PAGING_BASE + idx*PAGING_PAGE_SIZE)

using namespace Stage3::Paging;

/** Private declarations */
void
_init_vload();

int
_page_vmap(phys_addr_t paddr, virt_addr_t vaddr, bool user);

int
_page_vunmap(virt_addr_t vaddr);

/** The main table structure */
struct pml4t_entry* PML4T;
struct pdpt_entry* _VPDPT;
struct pd_entry* _VPD;
struct pt_entry* _VPT;

#define __vload(ref, addr)                              \
    {                                                   \
    ((struct pt_entry*) PAGING_BASE)[ref]  = pt_entry(  \
            (phys_addr_t)addr,                          \
            false,                                      \
            true, true,                                 \
            false, false, false, false, false           \
        );                                              \
    asm volatile("invlpg (%0);" : :                     \
    "A"(PAGING_BASE+ref*PAGING_PAGE_SIZE) : "memory");  \
    }

#define __vload_pdpt(addr)    __vload(VPAGE_PDPT, addr);
#define __vload_pd(addr)      __vload(VPAGE_PD, addr);
#define __vload_pt(addr)      __vload(VPAGE_PT, addr);


void 
Stage3::Paging::init()
{  
    PML4T = (struct pml4t_entry*) 0x101000;
    
    _init_vload();
    
    __vload(VPAGE_PDPT, 0x420000);
    *(uint32_t*)__VLOAD_PAGE(VPAGE_PDPT) = 0x20202020;
    
    // mappings for the 4 paging levels
    __vload(VPAGE_PML4T, PML4T);
        
    __vload_pdpt(0);
    __vload_pd(0);
    __vload_pt(0);
    
    PML4T = (struct pml4t_entry*)__VLOAD_PAGE(VPAGE_PML4T);
    _VPDPT = (struct pdpt_entry*)__VLOAD_PAGE(VPAGE_PDPT);
    _VPD = (struct pd_entry*)__VLOAD_PAGE(VPAGE_PD);
    _VPT = (struct pt_entry*)__VLOAD_PAGE(VPAGE_PT);
}


int
Stage3::Paging::vmap(phys_addr_t paddr, virt_addr_t vaddr, uint32_t npage, 
    bool user)
{
    paddr = PM_PAGE_ALIGN_INF(paddr);
    vaddr = PM_PAGE_ALIGN_INF(vaddr);
    
    //printf("VMAP %lx-%lx -> %lx\n", paddr, paddr+npage*PAGING_PAGE_SIZE-1, vaddr);
    while (npage)
    {
        if (_page_vmap(paddr, vaddr, user) < 0)
            return -1;
            
        paddr += PAGING_PAGE_SIZE;
        vaddr += PAGING_PAGE_SIZE;
        npage--;
    }
    
    return 0;
}

int
Stage3::Paging::vunmap(virt_addr_t vaddr, uint32_t npage)
{
    vaddr = PM_PAGE_ALIGN_INF(vaddr);
    //printf("VUNMAP %lx, %d pages\n", vaddr, npage);
    while (npage)
    {
        int r = _page_vunmap(vaddr);
        
        if (r < 0)
        {
            printf("woops at page %lx : err %d\n", vaddr, r);
            //return r;
        }
            
        vaddr += PAGING_PAGE_SIZE;
        npage--;
    }
    
    return 0;
}


/** Table initializers helpers */ 
struct pdpt_entry*
pdpt_entry::init()
{
    int idx;
    struct pdpt_entry* PDPT = (struct pdpt_entry*)Stage3::PhysicalMemoryAllocator::getpage();
    
    __vload_pdpt(PDPT);    
    for (idx = 0; idx < 512; idx++)
        _VPDPT[idx] = pdpt_entry();
       
    return PDPT;
}

struct pd_entry*
pd_entry::init()
{
    int idx;
    struct pd_entry* PD = (struct pd_entry*)Stage3::PhysicalMemoryAllocator::getpage();
    
    __vload_pd(PD);  
    for (idx = 0; idx < 512; idx++)
        _VPD[idx] = pd_entry();
        
    return PD;
}

struct pt_entry*
pt_entry::init()
{
    int idx;
    struct pt_entry* PT = (struct pt_entry*)Stage3::PhysicalMemoryAllocator::getpage();
    
    __vload_pt(PT);
    for (idx = 0; idx < 512; idx++)
        _VPT[idx] = pt_entry();
        
    return PT;
}


/** Sub-tables pointer helpers */
struct pdpt_entry*
pml4t_entry::ptr()
{
    return reinterpret_cast<struct pdpt_entry*>(
              (((phys_addr_t)base_lo & 0xF) << 12) 
            | (((phys_addr_t)base_mid & 0xFFFF) << 16) 
            | (((phys_addr_t)base_hi & 0xFF) << 32)
        );
}

struct pd_entry*
pdpt_entry::ptr()
{
    return reinterpret_cast<struct pd_entry*>(
              (((phys_addr_t)base_lo & 0xF) << 12) 
            | (((phys_addr_t)base_mid & 0xFFFF) << 16) 
            | (((phys_addr_t)base_hi & 0xFF) << 32)
        );
}

struct pt_entry*
pd_entry::ptr()
{
    return reinterpret_cast<struct pt_entry*>(
              (((phys_addr_t)base_lo & 0xF) << 12) 
            | (((phys_addr_t)base_mid & 0xFFFF) << 16) 
            | (((phys_addr_t)base_hi & 0xFF) << 32)
        );
}

phys_addr_t
pt_entry::ptr()
{
    return   (((phys_addr_t)base_lo & 0xFF) << 12) 
            | ((phys_addr_t)base_mid << 16) 
            | ((phys_addr_t)base_hi << 32);
}

/** Linear address helper */
linear_addr::linear_addr(virt_addr_t vaddr)
{
    offset         = ((vaddr) & 0x0000000000000FFF);
    table          = ((vaddr) & 0x00000000001FF000) >> 12;
    directory      = ((vaddr) & 0x000000003FE00000) >> 21;
    directory_ptr  = ((vaddr) & 0x0000007FC0000000) >> 30;
    pml4           = ((vaddr) & 0x0000FF8000000000) >> 39;
}

/** Private helpers */

int
_page_vmap(phys_addr_t paddr, virt_addr_t vaddr, bool user)
{
    struct linear_addr linear(vaddr);
    struct pml4t_entry *pml4te = &PML4T[PML4Toff(linear)];
    struct pdpt_entry *pdpt = NULL, *pdpte = NULL;
    struct pd_entry *pd = NULL, *pde = NULL;
    struct pt_entry *pt = NULL, *pte = NULL;
    
    // PML4T level
    if (!pml4te->isPresent())
    {
        pdpt = pdpt_entry::init();
        
        *pml4te = pml4t_entry(reinterpret_cast<phys_addr_t>(pdpt), 
                    PAGING_PAGE_PRESENT, PAGING_PAGE_RW, user, 
                    false, false, false);
    }
    else
        pdpt = pml4te->ptr();
        
    __vload_pdpt(pdpt);    
    pdpte = &_VPDPT[PDPToff(linear)];
    
    // PDPT level
    if (!pdpte->isPresent())
    {
        pd = pd_entry::init();
        
        *pdpte = pdpt_entry(reinterpret_cast<phys_addr_t>(pd), 
                    PAGING_PAGE_PRESENT, PAGING_PAGE_RW, user,
                    false, false, false);
    }
    else
        pd = pdpte->ptr();
        
    __vload_pd(pd);
    pde = &_VPD[PDoff(linear)];
    // PD level
    if (!pde->isPresent())
    {
        pt = pt_entry::init();
        
        *pde = pd_entry(reinterpret_cast<phys_addr_t>(pt), 
                PAGING_PAGE_PRESENT, PAGING_PAGE_RW, user,
                false, false, false, false);
    }
    else
        pt = pde->ptr();
        
    __vload_pt(pt);
    pte = &_VPT[PToff(linear)];
    
    // PT level
    if (!pte->isPresent() || pte->ptr() == paddr)
        *pte = pt_entry(paddr, false, 
            PAGING_PAGE_PRESENT, PAGING_PAGE_RW,
            user, false, false, false, false);
    else
    {
        printf("Warning: overwriting pte #%x, pde #%x, pdpte #%x, pml4te #%x\n",
            PToff(linear), PDoff(linear), PDPToff(linear), PML4Toff(linear));
    }
        
    return 0;
}


int
_page_vunmap(virt_addr_t vaddr)
{
    struct linear_addr linear(vaddr);
    struct pml4t_entry *pml4te = &PML4T[PML4Toff(linear)];
    struct pdpt_entry *pdpte = NULL;
    struct pd_entry *pde = NULL;
    struct pt_entry *pte = NULL;
    
    // PML4T level
    if (!pml4te->isPresent())
        return -1;
    else
        __vload_pdpt(pml4te->ptr());
    
    pdpte = &_VPDPT[PDPToff(linear)];
    
    // PDPT level
    if (!pdpte->isPresent())
        return -2;
    else
        __vload_pd(pdpte->ptr());
        
    pde = &_VPD[PDoff(linear)];
    
    // PD level
    if (!pde->isPresent())
        return -3;
    else
        __vload_pt(pde->ptr());
        
    pte = &_VPT[PToff(linear)];
    
    // PT level
    if (!pte->isPresent())
        return -4;
    else
        *pte = pt_entry();
    
    asm volatile("invlpg (%0);" : : "q"(vaddr) : "memory");
    return 0;
}


void
_init_vload()
{
    unsigned idx;
    struct linear_addr linear(VPAGE_PAGING_PT);
    struct pml4t_entry *pml4te = &PML4T[PML4Toff(linear)];
    struct pdpt_entry *pdpt = NULL, *pdpte = NULL;
    struct pd_entry *pd = NULL, *pde = NULL;
    struct pt_entry *pt = NULL, *pte = NULL;
    
    // PML4T level
    if (!pml4te->isPresent())
    {
        pdpt = (struct pdpt_entry*)Stage3::PhysicalMemoryAllocator::getpage();
 
        for (idx = 0; idx < 512; idx++)
            pdpt[idx] = pdpt_entry();
        
        *pml4te = pml4t_entry(reinterpret_cast<phys_addr_t>(pdpt), 
                    PAGING_PAGE_PRESENT, PAGING_PAGE_RW, false, 
                    false, false, false);
    }
    else
        pdpt = pml4te->ptr();
         
    pdpte = &pdpt[PDPToff(linear)];
    
    // PDPT level
    if (!pdpte->isPresent())
    {
        pd = (struct pd_entry*)Stage3::PhysicalMemoryAllocator::getpage();
 
        for (idx = 0; idx < 512; idx++)
            pd[idx] = pd_entry();
        
        *pdpte = pdpt_entry(reinterpret_cast<phys_addr_t>(pd), 
                    PAGING_PAGE_PRESENT, PAGING_PAGE_RW, false,
                    false, false, false);
    }
    else
        pd = pdpte->ptr();
        
    pde = &pd[PDoff(linear)];
    
    // PD level
    if (!pde->isPresent())
    {
        pt = (struct pt_entry*)Stage3::PhysicalMemoryAllocator::getpage();
 
        for (idx = 0; idx < 512; idx++)
            pt[idx] = pt_entry();
        
        *pde = pd_entry(reinterpret_cast<phys_addr_t>(pt), 
                PAGING_PAGE_PRESENT, PAGING_PAGE_RW, false,
                false, false, false, false);
    }
    else
        pt = pde->ptr();
        
    pte = &pt[PToff(linear)];
    
    // PT level
    *pte = pt_entry(reinterpret_cast<phys_addr_t>(pt), false, 
        PAGING_PAGE_PRESENT, PAGING_PAGE_RW,
        false, false, false, false, false);
}
