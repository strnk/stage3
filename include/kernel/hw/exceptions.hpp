#ifndef _HW_EXCEPTIONS_H_
#define _HW_EXCEPTIONS_H_

#ifndef ASM_SOURCE
# include <kernel/hw/cpu.h>
#endif

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

#define EXCEPTION_PF_PROTECTION_VIOLATION       0x01
#define EXCEPTION_PF_WRITE_ACCESS               0x02
#define EXCEPTION_PF_USER_MODE                  0x04
#define EXCEPTION_PF_RSVD                       0x08
#define EXCEPTION_PF_INSTRUCTION_FETCH          0x10

#ifndef ASM_SOURCE
namespace Stage3 {
namespace Exceptions {

    typedef void 
    (*handler_t)(uint64_t exception, 
        const struct cpu_context *context);

    void
    init(void);

    void
    define_handler(int exception, handler_t handler);

    void
    default_handler(uint64_t exception, 
        const struct cpu_context *context);
            
}
}
#endif // ASM_SOURCE

#endif // EXCEPTIONS
