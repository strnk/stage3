#include <kernel/hw/interrupts.h>
#include <kernel/hw/segments.h>
#include <stdlib.h>

/* Structures should be 8-byte aligned for caching */
static struct idt_register IDTR __align(0x10);
static struct idt_entry IDT[IDT_DESCRIPTOR_COUNT] __align(0x10);

void
idt_init(void)
{
    int i = 0;
    
    // Initialize an empty IDT
    for (i = 0; i < IDT_DESCRIPTOR_COUNT; i++)
        idt_init_descriptor(i, SEGMENT_SELECTOR(KSEGMENT_CODE, 0, 0), 
            IDTE_TYPE_INTERRUPT_GATE);
    
    IDTR.base_address = (uint64_t) IDT;
    IDTR.size = sizeof(IDT) - 1;
    
    // Load the IDT
    asm volatile("lidt %0" : : "m"(IDTR) : "memory");
}


void
idt_init_descriptor(uint8_t vector, uint16_t segment, uint8_t type)
{
    IDT[vector].selector = segment;
    IDT[vector].type = type;
    
    // Will be set by idt_set_handler
    IDT[vector].offset_lo = 0;
    IDT[vector].offset_mid = 0;
    IDT[vector].offset_hi = 0;
    IDT[vector].ist = 0;
    IDT[vector].dpl = 0;
    IDT[vector].present = IDTE_DISABLED;
    
    // Unused, initialize to zero
    IDT[vector].zero1 = 0;
    IDT[vector].zero2 = 0;
    IDT[vector].reserved = 0;
} 


void
idt_set_handler(uint8_t vector, virt_addr_t handler, uint8_t dpl)
{    
    if (handler != (virt_addr_t)NULL)
    {
        IDT[vector].offset_lo = handler & 0xFFFF;
        IDT[vector].offset_mid = (handler & 0xFFFF0000) >> 16;
        IDT[vector].offset_hi = (handler & 0xFFFFFFFF00000000) >> 32;
        IDT[vector].dpl = dpl;
        IDT[vector].present = IDTE_PRESENT;
    }
    else
    {
        IDT[vector].offset_lo = 0;
        IDT[vector].offset_mid = 0;
        IDT[vector].offset_hi = 0;
        IDT[vector].dpl = 0;
        IDT[vector].present = IDTE_DISABLED;
    }
}
