#include <kernel/hw/interrupts.hpp>
#include <kernel/hw/exceptions.hpp>
#include <kernel/hw/segments.h>
#include <stdlib.h>
#include <cstdio>

using namespace Stage3::Interrupts;

/* Structures should be 8-byte aligned for caching */
static struct IDT::idt_register IDTR __align(0x10);
static struct IDT::idt_entry IDTVectors[IDT_DESCRIPTOR_COUNT] __align(0x10);

Stage3::Interrupts::Manager *Stage3::Interrupts::GlobalManager;

extern uint64_t __interrupt_wrappers[INTERRUPTS_MASKABLE_COUNT];
interrupt_handler_t __interrupt_handlers[INTERRUPTS_MASKABLE_COUNT];

void
Stage3::Interrupts::init(void)
{
    int i;
    
    Stage3::Exceptions::init();     
    
    for (i = INTERRUPTS_MASKABLE_BASE; i < INTERRUPTS_MASKABLE_BASE+16; i++)
    {
        IDT::define_handler(i, 
            __interrupt_wrappers[i - INTERRUPTS_MASKABLE_BASE], 0);
        Stage3::Interrupts::define_handler(i, 
            Stage3::Interrupts::default_handler);
    }
}

void
Stage3::Interrupts::define_handler(uint8_t vector, interrupt_handler_t handler)
{
    __interrupt_handlers[vector - INTERRUPTS_MASKABLE_BASE] = handler;
}

void
Stage3::Interrupts::default_handler(uint64_t intNum)
{
    printf("!%ld", intNum);
}


void
IDT::init(void)
{
    int i = 0;
    
    // Initialize an empty IDT
    for (i = 0; i < IDT_DESCRIPTOR_COUNT; i++)
        IDTVectors[i].init(SEGMENT_SELECTOR(KSEGMENT_CODE, 0, 0), 
            IDTE_TYPE_INTERRUPT_GATE);
    
    IDTR = idt_register((uint64_t) IDTVectors, sizeof(IDTVectors));
    IDTR.load();
}

void
IDT::idt_register::load()
{
    // Load the IDT
    asm volatile("lidt (%0)" : : "p"(this) : "memory");
}

void
IDT::idt_entry::init(uint16_t segment, uint8_t type)
{
    selector = segment;
    this->type = type;
    
    // Will be set by set_handler
    offset_lo = 0;
    offset_mid = 0;
    offset_hi = 0;
    ist = 0;
    dpl = 0;
    present = IDTE_DISABLED;
    
    // Unused, initialize to zero
    zero1 = 0;
    zero2 = 0;
    reserved = 0;
}

void
IDT::idt_entry::set_handler(virt_addr_t handler, uint8_t dpl)
{
    offset_lo = handler & 0xFFFF;
    offset_mid = (handler & 0xFFFF0000) >> 16;
    offset_hi = (handler & 0xFFFFFFFF00000000) >> 32;
    dpl = dpl;
    present = IDTE_PRESENT;
}

void
IDT::idt_entry::disable()
{
    present = IDTE_DISABLED;
}


void
IDT::define_handler(uint8_t vector, virt_addr_t handler, uint8_t dpl)
{    
    if (handler != (virt_addr_t)NULL)
        IDTVectors[vector].set_handler(handler, dpl);
    else
        IDTVectors[vector].disable();
}


extern "C"
void __int_manager_eoi(uint64_t vector)
{
    GlobalManager->eoi(vector);
}
