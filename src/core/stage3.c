#include <multiboot.h>
#include <kernel/tty.h>
#include <kernel/tty/videomem.h>
#include <kernel/hw/msr.h>
#include <kernel/hw/apic.h>
#include <kernel/hw/interrupts.h>
#include <kernel/hw/interrupt_handlers.h>
#include <kernel/hw/segmentation.h>

#include <stdio.h>

extern int videomem_cursor_attr;

void main(unsigned long magic, multiboot_info_t* mbi)
{
    /** Console display initialization */
    tty_set_handler(&TTY_VIDEOMEM);
    puts("STAGE3 operating system");
    
    /** We _need_ multiboot data, halt here if we're not loaded with multiboot */
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
    {
        printf("No multiboot valid data found, halting.");
        return;
    }
    
    multiboot_dump(mbi);
    
    /** Setup the flat segmentation memory model */
    gdt_init();
    
    /** Interrupt initialization */
    idt_init();
    interrupt_init();
    
    //asm volatile("movl 0xdeadbeef, %%eax" : : : "eax");
    //asm volatile("int $3" : : );
    
    printf("  CPUID is %ssupported. (%d)\n", (cpuid_supported() != 0)?"":"not ", cpuid_supported());
    
    /** Interrupts initialization */
    init_apic(0);
    
    {
        
        printf("  APIC: \n    ` Base address: 0x%x\n", (uint32_t)APIC_BASE_ADDRESS);
        printf("    ` Spurious register: 0x%x\n", read_lapic_reg(APIC_REG_SPURIOUS));
        printf("    ` Version: 0x%x\n", read_lapic_reg(APIC_REG_VERSION));
        printf("    ` ID: 0x%x\n", read_lapic_reg(APIC_REG_ID));
        printf("    ` DFR: 0x%x\n", read_lapic_reg(APIC_REG_DFR));
    }
    
    /** Print end message */
    puts(COLOR(ISO6429_RED) "Halting.");
}
