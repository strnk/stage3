extern "C" {
#include <kernel/tty.h>
#include <kernel/tty/videomem.h>
#include <kernel/hw/ebda.h>

}

#include <kernel/hw/segmentation.hpp>
#include <kernel/hw/interrupts.hpp>
#include <kernel/hw/exceptions.hpp>
#include <kernel/hw/pm_alloc.hpp>
#include <kernel/hw/paging.hpp>
#include <kernel/hw/ioapic.hpp>
#include <kernel/hw/lapic.hpp>
#include <kernel/hw/kheap.hpp>
#include <kernel/hw/mp.hpp>
#include <kernel/hw/irq.h>
#include <kernel/hw/pic.h>
#include <kernel/die.hpp>
#include <multiboot.hpp>

#include <kernel/devices/keyboard.hpp>
#include <kernel/devices/8042.hpp>

#include <cstdio>

using namespace Stage3;

void main(unsigned long magic, Multiboot::info_t* mbi)
{
    /** 
     ** Setup the flat segmentation memory model 
     **/
    GDT::init();
    
    /** 
     ** Console display initialization 
     **/
    tty_set_handler(&TTY_VIDEOMEM);
    puts("STAGE3 operating system");


    /**
     ** Initialize exceptions vectors
     **/
    Interrupts::IDT::init();
    Interrupts::init();
    
    /** We _need_ multiboot data, halt here if we're not loaded with multiboot */
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
    {
        die("No multiboot valid data found, halting.");
        return;
    }
    
    /** 
     ** Physical memory allocator initialization
     **/
    PhysicalMemoryAllocator::init(mbi->find_phys_max());
    
    // Reserved memory pages from multiboot data
    Multiboot::memory_map_t::iterator it;
    for (it = mbi->mmap_begin(); it != mbi->mmap_end(); it++)
    {
        if (!it->isAvailable())
        {
            //printf("%lx -- %lx\n", it->addr, it->addr + it->len-1);
            PhysicalMemoryAllocator::mark_reserved(
                it->addr, 
                it->addr+it->len-1
            );
        }
    }
    
    // Bootloader used pages
    PhysicalMemoryAllocator::mark_reserved(0x1000, 0x120000-1);
    
    
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
    
    /**
     ** Kernel heap init
     **/
    HeapAllocator::init(1024);
    
    Devices::PS2Controller.init();
    
    /** EBDA initialization */
    ebda_init();
    
    /** MP Table initialization */
    MP::init();
    
    /** Interrupts manager initialization */
    if (MP::hasIOAPIC())
    {
        puts("Interrupts: IOAPIC detected, using APIC for interrupts");
        // Have to disable PIC to prevent IRQ coming from multiple sources
        init_pic();
        disable_pic();
        
        Interrupts::GlobalManager = new Interrupts::APICManager();
    }
    else
    {
        puts("Interrupts: using PIC for interrupts (STUB)");
        // PIC initialization
    }
    
    Interrupts::GlobalManager->init();
    
    Interrupts::define_handler(0x20 + IRQ_ISA_KEYBOARD,
        Devices::_PS2Controller::_irq1);
    Interrupts::define_handler(0x20 + IRQ_ISA_PS2, 
        Devices::_PS2Controller::_irq12);
    
    Interrupts::GlobalManager->map(Interrupts::Manager::BUS_ISA, 
        IRQ_ISA_KEYBOARD, 0x20 + IRQ_ISA_KEYBOARD);
    Interrupts::GlobalManager->map(Interrupts::Manager::BUS_ISA, 
        IRQ_ISA_PS2, 0x20 + IRQ_ISA_PS2);
    
    Interrupts::GlobalManager->enable(Interrupts::Manager::BUS_ISA,
        IRQ_ISA_KEYBOARD);
    Interrupts::GlobalManager->enable(Interrupts::Manager::BUS_ISA, 
        IRQ_ISA_PS2);
    
    __enable_interrupts();
    Devices::PS2Controller.autoDetect();
    Devices::PS2Controller.enableIRQ();
    
    /** Print end message */
    printf(COLOR(ISO6429_RED) "\nHalting.");
    for (;;)
        asm volatile("hlt" : : : "memory");
}
