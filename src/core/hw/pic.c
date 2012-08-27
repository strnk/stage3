#include <kernel/hw/pic.h>
#include <kernel/hw/io.h>

void
init_pic(void)
{
    // Send ICW1
    outb(PIC_MASTER_COMMAND, 
        PIC_ICW1 | PIC_WITH_ICW4 | PIC_CASCADE_MODE | PIC_EDGE_TRIGGERED);
    outb(PIC_SLAVE_COMMAND, 
        PIC_ICW1 | PIC_WITH_ICW4 | PIC_CASCADE_MODE | PIC_EDGE_TRIGGERED);
        
    // Program the base interrupt vectors (ICW2)
    outb(PIC_MASTER_DATA, PIC_ICW2 | 0x20);
    outb(PIC_SLAVE_DATA, PIC_ICW2 | 0x28);
    
    // Configure the cascade mapping (ICW3)
    outb(PIC_MASTER_DATA, PIC_ICW3 | PIC_SLAVE_ON_IR(PIC_CASCADE_WIRE));
    outb(PIC_SLAVE_DATA, PIC_ICW3 | PIC_SLAVE_WIRED_TO(PIC_CASCADE_WIRE));
    
    // Configuration word (ICW4)
    outb(PIC_MASTER_DATA, 
        PIC_ICW4 | PIC_8086_8088_MODE | PIC_NORMAL_EOI 
      | PIC_NON_BUFFERED | PIC_NOT_FULLY_NESTED);
    outb(PIC_SLAVE_DATA, 
        PIC_ICW4 | PIC_8086_8088_MODE | PIC_NORMAL_EOI 
      | PIC_NON_BUFFERED | PIC_NOT_FULLY_NESTED);
      
    pic_set_irq_mask(0xFFFF);
}

void
disable_pic(void)
{
    pic_set_irq_mask(0xFFFF);
}


inline void
pic_set_irq_mask(uint16_t mask)
{
    outb(PIC_MASTER_DATA, PIC_OCW1 | (mask & 0x00FB));
    outb(PIC_SLAVE_DATA, PIC_OCW1 | ((mask & 0xFF00) >> 8));
}

void
pic_enable_irq(uint8_t irqLine)
{
    uint8_t previousState = 0;
    
    if (irqLine < 8)
    {
        previousState = inb(PIC_MASTER_DATA);
        outb(PIC_MASTER_DATA, PIC_OCW1 | (previousState & ~(1 << irqLine)));
    } 
    else if (irqLine > 15)
        return;
    else
    {
        previousState = inb(PIC_SLAVE_DATA);
        outb(PIC_MASTER_DATA, PIC_OCW1 | (previousState & ~(1 << (irqLine - 8))));
    }
}

void
pic_disable_irq(uint8_t irqLine)
{
    uint8_t previousState = 0;
    
    if (irqLine == PIC_CASCADE_WIRE)
        return;
        
    if (irqLine < 8)
    {
        previousState = inb(PIC_MASTER_DATA);
        outb(PIC_MASTER_DATA, PIC_OCW1 | previousState | (1 << irqLine));
    } 
    else if (irqLine > 15)
        return;
    else
    {
        previousState = inb(PIC_SLAVE_DATA);
        outb(PIC_MASTER_DATA, PIC_OCW1 | previousState | (1 << (irqLine - 8)));
    }
}
