#include <kernel/hw/paging.h>
#include <kernel/hw/pm_alloc.h>

#include <stdio.h>

#define PML4T_ENTRY(p, r, u, pw, pc, base, nx)              \
    ((struct pml4t_entry) {                                 \
        .present    = p,                                    \
        .rw         = r,                                    \
        .user       = u,                                    \
        .pwt        = pw,                                   \
        .pcd        = pc,                                   \
        .accessed   = 0,                                    \
        .reserved1  = 0,                                    \
        .__avail1   = 0,                                    \
        .base_lo    = ((base) & 0x000000000000F000) >> 12,  \
        .base_mid   = ((base) & 0x00000000FFFF0000) >> 16,  \
        .base_hi    = ((base) & 0x000000FF00000000) >> 32,  \
        .reserved2  = 0,                                    \
        .__avail2   = 0,                                    \
        .noexec     = 0                                     \
    })
    
#define PDPT_ENTRY(p, r, u, pw, pc, base, nx)              \
    ((struct pdpt_entry) {                                 \
        .present    = p,                                    \
        .rw         = r,                                    \
        .user       = u,                                    \
        .pwt        = pw,                                   \
        .pcd        = pc,                                   \
        .accessed   = 0,                                    \
        .reserved1  = 0,                                    \
        .__avail1   = 0,                                    \
        .base_lo    = ((base) & 0x000000000000F000) >> 12,  \
        .base_mid   = ((base) & 0x00000000FFFF0000) >> 16,  \
        .base_hi    = ((base) & 0x000000FF00000000) >> 32,  \
        .reserved2  = 0,                                    \
        .__avail2   = 0,                                    \
        .noexec     = 0                                     \
    })
    
#define PD_ENTRY(p, r, u, pw, pc, s, base, nx)           \
    ((struct pd_entry) {                                  \
        .present    = p,                                    \
        .rw         = r,                                    \
        .user       = u,                                    \
        .pwt        = pw,                                   \
        .pcd        = pc,                                   \
        .accessed   = 0,                                    \
        .reserved1  = 0,                                    \
        .size       = s,                                    \
        .reserved2  = 0,                                    \
        .__avail1   = 0,                                    \
        .base_lo    = ((base) & 0x000000000000F000) >> 12,  \
        .base_mid   = ((base) & 0x00000000FFFF0000) >> 16,  \
        .base_hi    = ((base) & 0x000000FF00000000) >> 32,  \
        .reserved3  = 0,                                    \
        .__avail2   = 0,                                    \
        .noexec     = 0                                     \
    })
    
#define PT_ENTRY(p, r, u, pw, pc, pa, g, base, nx)          \
    ((struct pt_entry) {                                  \
        .present    = p,                                    \
        .rw         = r,                                    \
        .user       = u,                                    \
        .pwt        = pw,                                   \
        .pcd        = pc,                                   \
        .dirty      = 0,                                    \
        .pat        = pa,                                   \
        .global     = g,                                    \
        .__avail1   = 0,                                    \
        .base_lo    = ((base) & 0x000000000000F000) >> 12,  \
        .base_mid   = ((base) & 0x00000000FFFF0000) >> 16,  \
        .base_hi    = ((base) & 0x000000FF00000000) >> 32,  \
        .reserved1  = 0,                                    \
        .__avail2   = 0,                                    \
        .noexec     = 0                                     \
    })
    

/** Private declarations */
void
_init_vload();

int
_page_vmap(phys_addr_t paddr, virt_addr_t vaddr, uint8_t user);

int
_page_vunmap(virt_addr_t vaddr);

/** The main table structure */
struct pml4t_entry* PML4T;
struct pdpt_entry* _VPDPT;
struct pd_entry* _VPD;
struct pt_entry* _VPT;

#define __vload(ref, addr)                              \
    {                                                   \
    ((struct pt_entry*) PAGING_BASE)[ref]  = PT_ENTRY(  \
            PAGING_PAGE_PRESENT,                        \
            PAGING_PAGE_RW,                             \
            0, 0, 0, 0, 0,                              \
            (phys_addr_t)addr,                          \
            0                                           \
        );                                              \
    asm volatile("invlpg (%0);" : :                     \
    "A"(PAGING_BASE+ref*PAGING_PAGE_SIZE) : "memory");  \
    }

#define __vload_pdpt(addr)    __vload(VPAGE_PDPT, addr);
#define __vload_pd(addr)      __vload(VPAGE_PD, addr);
#define __vload_pt(addr)      __vload(VPAGE_PT, addr);


void 
paging_init()
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
paging_vmap(phys_addr_t paddr, virt_addr_t vaddr, uint32_t npage, uint8_t user)
{
    printf("VMAP %lx -> %lx, %d pages\n", paddr, vaddr, npage);
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
paging_vunmap(virt_addr_t vaddr, uint32_t npage)
{
    printf("VUNMAP %lx, %d pages\n", vaddr, npage);
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
pdpt_init()
{
    int idx;
    struct pdpt_entry* PDPT = (struct pdpt_entry*)pm_alloc_getpage();
    
    for (idx = 0; idx < 512; idx++)
        PDPT[idx] = PDPT_ENTRY(0, 0, 0, 0, 0, 0, 0);
       
    return PDPT;
}

struct pd_entry*
pd_init()
{
    int idx;
    struct pd_entry* PD = (struct pd_entry*)pm_alloc_getpage();
    
    for (idx = 0; idx < 512; idx++)
        PD[idx] = PD_ENTRY(0, 0, 0, 0, 0, 0, 0, 0);
        
    return PD;
}

struct pt_entry*
pt_init()
{
    int idx;
    struct pt_entry* PT = (struct pt_entry*)pm_alloc_getpage();
    
    for (idx = 0; idx < 512; idx++)
        PT[idx] = PT_ENTRY(0, 0, 0, 0, 0, 0, 0, 0, 0);
        
    return PT;
}

/** Private helpers */

int
_page_vmap(phys_addr_t paddr, virt_addr_t vaddr, uint8_t user)
{
    struct linear_addr linear = VADDR_TO_LINEAR(vaddr);
    struct pml4t_entry *pml4te = &PML4T[PML4Toff(linear)];
    struct pdpt_entry *pdpt = NULL, *pdpte = NULL;
    struct pd_entry *pd = NULL, *pde = NULL;
    struct pt_entry *pt = NULL, *pte = NULL;
    
    // PML4T level
    if (!(pml4te->present & PAGING_PAGE_PRESENT))
    {
        pdpt = pdpt_init();
        
        *pml4te = PML4T_ENTRY(
            PAGING_PAGE_PRESENT,
            PAGING_PAGE_RW,
            user,
            0, 0,
            (phys_addr_t)pdpt,
            0
        );
    }
    else
        pdpt = PDPT_ADDR(pml4te);
        
    __vload_pdpt(pdpt);    
    pdpte = &_VPDPT[PDPToff(linear)];
    
    // PDPT level
    if (!(pdpte->present & PAGING_PAGE_PRESENT))
    {
        pd = pd_init();
        
        *pdpte = PDPT_ENTRY(
            PAGING_PAGE_PRESENT,
            PAGING_PAGE_RW,
            user,
            0, 0,
            (phys_addr_t)pd,
            0
        );
    }
    else
        pd = PD_ADDR(pdpte);
        
    __vload_pd(pd);
    pde = &_VPD[PDoff(linear)];
    
    // PD level
    if (!(pde->present & PAGING_PAGE_PRESENT))
    {
        pt = pt_init();
        
        *pde = PD_ENTRY(
            PAGING_PAGE_PRESENT,
            PAGING_PAGE_RW,
            user,
            0, 0, 0,
            (phys_addr_t)pt,
            0
        );
    }
    else
        pt = PT_ADDR(pde);
        
    __vload_pt(pt);
    pte = &_VPT[PToff(linear)];
    
    // PT level
    if (!(pte->present & PAGING_PAGE_PRESENT))
        *pte = PT_ENTRY(
            PAGING_PAGE_PRESENT,
            PAGING_PAGE_RW,
            user,
            0, 0, 0, 0,
            paddr,
            0
        );
        
    return 0;
}


int
_page_vunmap(virt_addr_t vaddr)
{
    struct linear_addr linear = VADDR_TO_LINEAR(vaddr);
    struct pml4t_entry *pml4te = &PML4T[PML4Toff(linear)];
    struct pdpt_entry *pdpte = NULL;
    struct pd_entry *pde = NULL;
    struct pt_entry *pte = NULL;
    
    // PML4T level
    if (!(pml4te->present & PAGING_PAGE_PRESENT))
        return -1;
    else
        __vload_pdpt(PDPT_ADDR(pml4te));
    
    pdpte = &_VPDPT[PDPToff(linear)];
    
    // PDPT level
    if (!(pdpte->present & PAGING_PAGE_PRESENT))
        return -2;
    else
        __vload_pd(PD_ADDR(pdpte));
        
    pde = &_VPD[PDoff(linear)];
    
    // PD level
    if (!(pde->present & PAGING_PAGE_PRESENT))
        return -3;
    else
        __vload_pt(PT_ADDR(pde));
        
    pte = &_VPT[PToff(linear)];
    
    // PT level
    if (!(pte->present & PAGING_PAGE_PRESENT))
        return -4;
    else
        *pte = PT_ENTRY(0, 0, 0, 0, 0, 0, 0, 0, 0);
    
    asm volatile("invlpg (%0);" : : "q"(vaddr) : "memory");
    return 0;
}


void
_init_vload()
{
    struct linear_addr linear = VADDR_TO_LINEAR(VPAGE_PAGING_PT);
    struct pml4t_entry *pml4te = &PML4T[PML4Toff(linear)];
    struct pdpt_entry *pdpt = NULL, *pdpte = NULL;
    struct pd_entry *pd = NULL, *pde = NULL;
    struct pt_entry *pt = NULL, *pte = NULL;
    
    // PML4T level
    if (!(pml4te->present & PAGING_PAGE_PRESENT))
    {
        pdpt = pdpt_init();
        
        *pml4te = PML4T_ENTRY(
            PAGING_PAGE_PRESENT,
            PAGING_PAGE_RW,
            0,
            0, 0,
            (phys_addr_t)pdpt,
            0
        );
    }
    else
        pdpt = PDPT_ADDR(pml4te);
         
    pdpte = &pdpt[PDPToff(linear)];
    
    // PDPT level
    if (!(pdpte->present & PAGING_PAGE_PRESENT))
    {
        pd = pd_init();
        
        *pdpte = PDPT_ENTRY(
            PAGING_PAGE_PRESENT,
            PAGING_PAGE_RW,
            0,
            0, 0,
            (phys_addr_t)pd,
            0
        );
    }
    else
        pd = PD_ADDR(pdpte);
        
    pde = &pd[PDoff(linear)];
    
    // PD level
    if (!(pde->present & PAGING_PAGE_PRESENT))
    {
        pt = pt_init();
        
        *pde = PD_ENTRY(
            PAGING_PAGE_PRESENT,
            PAGING_PAGE_RW,
            0,
            0, 0, 0,
            (phys_addr_t)pt,
            0
        );
    }
    else
        pt = PT_ADDR(pde);
        
    pte = &pt[PToff(linear)];
    
    // PT level
    *pte = PT_ENTRY(
        PAGING_PAGE_PRESENT,
        PAGING_PAGE_RW,
        0,
        0, 0, 0, 0,
        (phys_addr_t)pt,
        0
    );
}
