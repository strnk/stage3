#include <multiboot.h>
#include <kernel/tty.h>
#include <kernel/tty/videomem.h>
#include <kernel/hw/msr.h>
#include <kernel/hw/apic.h>
#include <kernel/hw/interrupts.h>
#include <kernel/hw/interrupt_handlers.h>
#include <kernel/hw/segmentation.h>
#include <kernel/hw/pm_alloc.h>

#include <stdio.h>

extern int videomem_cursor_attr;

uint64_t memsize;

void main(unsigned long magic, multiboot_info_t* mbi)
{
    /** Setup the flat segmentation memory model */
    gdt_init();
    idt_init();
    interrupt_init();
    
    /** Console display initialization */
    tty_set_handler(&TTY_VIDEOMEM);
    puts("STAGE3 operating system");
    
    /** We _need_ multiboot data, halt here if we're not loaded with multiboot */
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
    {
        printf("No multiboot valid data found, halting.");
        return;
    }
    
    puts("Data size static test:\n");
    printf("uint64_t: %ld\n", sizeof(uint64_t));
    printf("uint32_t: %ld\n", sizeof(uint32_t));
    printf("uint16_t: %ld\n", sizeof(uint16_t));
    printf("uint8_t: %ld\n", sizeof(uint8_t));
    printf("idt_entry: %ld\n", sizeof(struct idt_entry));
    

    //multiboot_dump(mbi);
    
    /** Physical memory allocator initialization */
    //pm_alloc_init(multiboot_find_phys_max(mbi));
    
    /*multiboot_memory_map_t *mmap;
    for (mmap = (multiboot_memory_map_t*)(uint64_t)mbi->mmap_addr;
        (uint64_t)mmap < mbi->mmap_addr + mbi->mmap_length;
        mmap++)
    {
        if (mmap->type != MULTIBOOT_MEMORY_AVAILABLE)
            pm_alloc_mark_reserved(mmap->addr, mmap->addr+mmap->len);
    }*/
    
    //pm_state_dump();
    
    
    /** Interrupt initialization */
    //idt_init();
    //interrupt_init();
    
    //asm volatile("movl 0xdeadbeef, %%eax" : : : "eax");
    //asm volatile("int $3" : : );
    
    printf("  CPUID is %ssupported. (%d)\n", (cpuid_supported() != 0)?"":"not ", cpuid_supported());
    
    /** Interrupts initialization */
    //init_apic(0);
    /*
    {
        
        printf("  APIC: \n    ` Base address: 0x%x\n", (uint32_t)APIC_BASE_ADDRESS);
        printf("    ` Spurious register: 0x%x\n", read_lapic_reg(APIC_REG_SPURIOUS));
        printf("    ` Version: 0x%x\n", read_lapic_reg(APIC_REG_VERSION));
        printf("    ` ID: 0x%x\n", read_lapic_reg(APIC_REG_ID));
        printf("    ` DFR: 0x%x\n", read_lapic_reg(APIC_REG_DFR));
    }
    */
    
    /** Print end message */
    puts(COLOR(ISO6429_RED) "Halting.");
}
