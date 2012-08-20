#include <multiboot.h>
#include <kernel/tty.h>
#include <kernel/tty/videomem.h>
#include <kernel/hw/msr.h>
#include <kernel/hw/apic.h>
#include <kernel/hw/interrupts.h>
#include <kernel/hw/interrupt_handlers.h>
#include <kernel/hw/segmentation.h>
#include <kernel/hw/pm_alloc.h>
#include <kernel/hw/paging.h>
#include <kernel/die.h>

#include <stdio.h>

extern int videomem_cursor_attr;

uint64_t memsize;

void main(unsigned long magic, multiboot_info_t* mbi)
{
    /** 
     ** Setup the flat segmentation memory model 
     **/
    gdt_init();
    
    /**
     ** Initialize exceptions vectors
     **/
    idt_init();
    interrupt_init();
    
    /** 
     ** Console display initialization 
     **/
    tty_set_handler(&TTY_VIDEOMEM);
    puts("STAGE3 operating system");
    
    /** We _need_ multiboot data, halt here if we're not loaded with multiboot */
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
    {
        printf("No multiboot valid data found, halting.");
        return;
    }
    
    /** 
     ** Physical memory allocator initialization
     **/
    pm_alloc_init(multiboot_find_phys_max(mbi));
    
    // Reserved memory pages from multiboot data
    multiboot_memory_map_t *mmap;
    for (mmap = (multiboot_memory_map_t*)(uint64_t)mbi->mmap_addr;
        (uint64_t)mmap < mbi->mmap_addr + mbi->mmap_length;
        mmap++)
    {
        if (mmap->type != MULTIBOOT_MEMORY_AVAILABLE)
        {
            printf("%lx -- %lx\n", mmap->addr, mmap->addr+mmap->len-1);
            pm_alloc_mark_reserved(mmap->addr, mmap->addr+mmap->len-1);
        }
    }
    
    // Bootloader used pages
    pm_alloc_mark_reserved(0x1000, 0x120000);
    pm_state_dump();
    
    
    /**
     ** Paging initialization
     **/
    paging_init();
    
    /**
     ** Final page mapping
     **/
    {
        /* Remap the kernel stack */
        phys_addr_t rsp, rbp;
        asm volatile("movq %%rsp, %0; mov %%rbp, %1" : "=r"(rsp), "=r"(rbp) : : "memory");
        
        paging_vmap((phys_addr_t)0x110000, (virt_addr_t)0xfffffffe80000000, 4, 
            PAGING_PAGE_SUPERVISOR);
            
        rsp = (rsp-0x110000)+0xfffffffe80000000;
        rbp = (rbp-0x110000)+0xfffffffe80000000;
        
        asm volatile("movq %0, %%rsp; movq %1, %%rbp" : : "m"(rsp), "m"(rbp) : "memory");
        
        /* Unmap identity paged low-memory after 1MB */
        paging_vunmap((virt_addr_t)0x100000, 256);
        paging_vunmap((virt_addr_t)0x200000, 512);
    }
    
    
    printf("CPUID is %ssupported.\n", (cpuid_supported() != 0)?"":"not ");
    
    /** Interrupts initialization */
    init_apic(0);
    {
        
        printf("  APIC: \n    ` Base address: 0x%lx\n", (uintptr_t)APIC_BASE_ADDRESS);
        printf("    ` Spurious register: 0x%x\n", read_lapic_reg(APIC_REG_SPURIOUS));
        printf("    ` Version: 0x%x\n", read_lapic_reg(APIC_REG_VERSION));
        printf("    ` ID: 0x%x\n", read_lapic_reg(APIC_REG_ID));
        printf("    ` DFR: 0x%x\n", read_lapic_reg(APIC_REG_DFR));
    }
    
    /** Print end message */
    puts(COLOR(ISO6429_RED) "Halting.");
    
    for (;;)
        asm volatile("hlt" : : : "memory");
}
