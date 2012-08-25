#ifndef _HW_PIC_H_
#define _HW_PIC_H_

#include <stdlib.h>
#include <inttypes.h>
#include <sys/cdefs.h>

// Ports used by the two i8259 PICs
#define PIC_MASTER_COMMAND  0x0020
#define PIC_MASTER_DATA     0x0021

#define PIC_SLAVE_COMMAND   0x00A0
#define PIC_SLAVE_DATA      0x00A1

#define PIC_CASCADE_WIRE    0x02


/*
 * Initialization Command Words
 */
 
// ICW1 (command)
#define PIC_ICW1                0x10

// ICW4 is mandatory for 8086 compatibility
#define PIC_WITH_ICW4           0x01
#define PIC_WITHOUT_ICW4        0x00

// Single: no other PIC in the system (no ICW3 needed)
#define PIC_SINGLE_MODE         0x02
#define PIC_CASCADE_MODE        0x00

// Adress interval for MCS-80, not needed for 8086
#define PIC_ADDRESS_INTERVAL_4  0x04
#define PIC_ADDRESS_INTERVAL_8  0x00

#define PIC_LEVEL_TRIGGERED     0x08
#define PIC_EDGE_TRIGGERED      0x00

// ICW2 (data)
#define PIC_ICW2                0x00

// ICW3 (data)
#define PIC_ICW3                0x00
#define PIC_SLAVE_ON_IR(n)      (1 << n)
#define PIC_SLAVE_WIRED_TO(n)   (n & 0x07)

// ICW4 (data)
#define PIC_ICW4                0x00

#define PIC_8086_8088_MODE      0x01
#define PIC_MCS80_85_MODE       0x00

#define PIC_AUTO_EOI            0x02
#define PIC_NORMAL_EOI          0x00

#define PIC_BUFFERED_MASTER     0xC0
#define PIC_BUFFERED_SLAVE      0x80
#define PIC_NON_BUFFERED        0x00

#define PIC_FULLY_NESTED        0x10
#define PIC_NOT_FULLY_NESTED    0x00


/*
 * Operation Command Words
 */

// OCW1 (command)
#define PIC_OCW1                0x00

// OCW2 (data)
#define PIC_OCW2                0x00
#define PIC_NON_SPECIFIC_EOI    0x10
#define PIC_SPECIFIC_EOI        0x30
#define PIC_NOP                 0x20

// OCW3 (data)
#define PIC_OCW3                0x08
#define PIC_READ_IR_REG         0x02
#define PIC_READ_IS_REG         0x03
#define PIC_POLL                0x04

__BEGIN_DECLS

void
init_pic(void);

void
disable_pic(void);

inline void
pic_set_irq_mask(uint16_t mask);


void
pic_enable_irq(uint8_t irqLine);


void
pic_disable_irq(uint8_t irqLine);


__END_DECLS

#endif // _HW_PIC_H_
