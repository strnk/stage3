#ifndef _HW_INTERRUPTS_H_
#define _HW_INTERRUPTS_H_

#ifndef ASM_SOURCE
# include <kernel/types.h>
#endif

/* IDT entry type */
#define IDTE_TYPE_TASK_GATE             0x05    
#define IDTE_TYPE_INTERRUPT_GATE        0x06
#define IDTE_TYPE_TRAP_GATE             0x07 
 
#define IDTE_SIZE_16BITS                0x00 
#define IDTE_SIZE_32BITS                0x01  

#define IDTE_PRESENT                    0x01
#define IDTE_DISABLED                   0x00 /* For convenience only ;) */

// Number of max. interrupt vectors
#define IDT_DESCRIPTOR_COUNT            256

// Interrupt routines
#define __disable_interrupts() asm volatile("cli" : : )
#define __enable_interrupts() asm volatile("sti" : : )

// Interrupts types
#define INTERRUPTS_EXCEPTION_BASE   0
#define INTERRUPTS_EXCEPTION_COUNT  32

#define INTERRUPTS_MASKABLE_BASE    32
#define INTERRUPTS_MASKABLE_COUNT   224

// Exception list
#define EXCEPTION_DIVIDE_ERROR                  0
#define EXCEPTION_DEBUG                         1
#define EXCEPTION_NMI_INTERRUPT                 2
#define EXCEPTION_BREAKPOINT                    3
#define EXCEPTION_OVERFLOW                      4
#define EXCEPTION_BOUND_RANGE_EXCEEDED          5
#define EXCEPTION_INVALID_OPCODE                6
#define EXCEPTION_DEVICE_NOT_AVAILABLE          7
#define EXCEPTION_DOUBLE_FAULT                  8
#define EXCEPTION_COPROCESSOR_SEGMENT_OVERRUN   9
#define EXCEPTION_INVALID_TSS                   10
#define EXCEPTION_SEGMENT_NOT_PRESENT           11
#define EXCEPTION_STACK_SEGMENT_FAULT           12
#define EXCEPTION_GENERAL_PROTECTION_FAULT      13
#define EXCEPTION_PAGE_FAULT                    14
#define EXCEPTION_RESERVED_1                    15
#define EXCEPTION_FLOATING_POINT_ERROR          16
#define EXCEPTION_ALIGNEMENT_CHECK              17
#define EXCEPTION_MACHINE_CHECK                 18
#define EXCEPTION_SIMD_FLOATING_POINT_EXCEPTION 19
#define EXCEPTION_RESERVED_2                    20
#define EXCEPTION_RESERVED_3                    21
#define EXCEPTION_RESERVED_4                    22
#define EXCEPTION_RESERVED_5                    23
#define EXCEPTION_RESERVED_6                    24
#define EXCEPTION_RESERVED_7                    25
#define EXCEPTION_RESERVED_8                    26
#define EXCEPTION_RESERVED_9                    27
#define EXCEPTION_RESERVED_10                   28
#define EXCEPTION_RESERVED_11                   29
#define EXCEPTION_RESERVED_12                   30
#define EXCEPTION_RESERVED_13                   31

#ifndef ASM_SOURCE
// Structure of an IDT entry
struct idt_entry {
    uint16_t    offset_lo;  // Offset[0..15]
    uint16_t    selector;   // Code segment in GDT
    uint8_t     reserved;   // Unused
    uint8_t     type : 3;
    uint8_t     size : 1;
    uint8_t     zero : 1;
    uint8_t     dpl : 2;
    uint8_t     present : 1;
    uint16_t    offset_hi;  // Offset[16..31]
} __packalign(8);

// Structure of the IDT register
struct idt_register {
    uint16_t    size;
    uint32_t    base_address;
} __pack;

void
idt_init(void);

void
idt_init_descriptor(uint8_t vector, uint16_t segment, uint8_t type, uint8_t size);

void
idt_set_handler(uint8_t vector, virt_addr_t handler, uint8_t dpl);
#endif // ASM_SOURCE

#endif // _HW_INTERRUPTS_H_
