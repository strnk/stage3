extern "C" {
#include <multiboot.h>
#include <kernel/tty.h>
#include <kernel/tty/videomem.h>
#include <kernel/hw/msr.h>
#include <kernel/hw/ebda.h>
#include <kernel/hw/ioapic.h>

}

#include <kernel/hw/segmentation.hpp>
#include <kernel/hw/interrupts.hpp>
#include <kernel/hw/exceptions.hpp>
#include <kernel/hw/pm_alloc.hpp>
#include <kernel/hw/paging.hpp>
#include <kernel/hw/lapic.hpp>
#include <kernel/die.hpp>

#include <cstdio>

using namespace Stage3;

void main(unsigned long magic, multiboot_info_t* mbi)
{
    /** 
     ** Setup the flat segmentation memory model 
     **/
    GDT::init();
    
    /**
     ** Initialize exceptions vectors
     **/
    Interrupts::IDT::init();
    Interrupts::init();
    
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
    PhysicalMemoryAllocator::init(multiboot_find_phys_max(mbi));
    
    // Reserved memory pages from multiboot data
    multiboot_memory_map_t *mmap;
    for (mmap = (multiboot_memory_map_t*)(uint64_t)mbi->mmap_addr;
        (uint64_t)mmap < mbi->mmap_addr + mbi->mmap_length;
        mmap++)
    {
        if (mmap->type != MULTIBOOT_MEMORY_AVAILABLE)
        {
            printf("%lx -- %lx\n", mmap->addr, mmap->addr+mmap->len-1);
            PhysicalMemoryAllocator::mark_reserved(mmap->addr, 
                mmap->addr+mmap->len-1);
        }
    }
    
    // Bootloader used pages
    PhysicalMemoryAllocator::mark_reserved(0x1000, 0x120000);
    PhysicalMemoryAllocator::state_dump();
    
    
    /**
     ** Paging initialization
     **/
    Paging::init();
    
    /**
     ** Final page mapping
     **/
    {
        /* Remap the kernel stack */
        phys_addr_t rsp, rbp;
        asm volatile("movq %%rsp, %0; mov %%rbp, %1" 
            : "=r"(rsp), "=r"(rbp) : : "memory");
        
        Paging::vmap((phys_addr_t)0x110000, (virt_addr_t)0xfffffffe80000000, 4, 
            PAGING_PAGE_SUPERVISOR);
            
        rsp = (rsp-0x110000)+0xfffffffe80000000;
        rbp = (rbp-0x110000)+0xfffffffe80000000;
        
        asm volatile("movq %0, %%rsp; movq %1, %%rbp" : : "m"(rsp), "m"(rbp) : "memory");
        
        /* Unmap identity paged low-memory after 1MB */
        Paging::vunmap((virt_addr_t)0x100000, 256);
        Paging::vunmap((virt_addr_t)0x200000, 512);
    }
    
    
    printf("CPUID is %ssupported.\n", (cpuid_supported() != 0)?"":"not ");
    
    /** EBDA initialization */
    ebda_init();
    
    /** Interrupts initialization */
    /*init_apic(0);
    {
        
        printf("  APIC: \n    ` Base address: 0x%lx\n",
            (uintptr_t)APIC_BASE_ADDRESS);
        printf("    ` Spurious register: 0x%x\n", 
            read_lapic_reg(APIC_REG_SPURIOUS));
        printf("    ` Version: 0x%x\n", read_lapic_reg(APIC_REG_VERSION));
        printf("    ` ID: 0x%x\n", read_lapic_reg(APIC_REG_ID));
        printf("    ` DFR: 0x%x\n", read_lapic_reg(APIC_REG_DFR));
    }*/
    
    /** Print end message */
    puts(COLOR(ISO6429_RED) "Halting.");
    
    for (;;)
        asm volatile("hlt" : : : "memory");
}
