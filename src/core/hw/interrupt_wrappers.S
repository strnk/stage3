#include <kernel/hw/interrupts.h>

.file "interrupt_wrappers.S"

.global __exception_wrappers
.extern __exception_handlers

.text

/* Generic wrapper for exception without error code */
.irp exceptionNum, \
    EXCEPTION_DIVIDE_ERROR, \
    EXCEPTION_DEBUG, \
    EXCEPTION_NMI_INTERRUPT, \
    EXCEPTION_BREAKPOINT, \
    EXCEPTION_OVERFLOW, \
    EXCEPTION_BOUND_RANGE_EXCEEDED, \
    EXCEPTION_INVALID_OPCODE, \
    EXCEPTION_DEVICE_NOT_AVAILABLE, \
    EXCEPTION_COPROCESSOR_SEGMENT_OVERRUN, \
    EXCEPTION_RESERVED_1, \
    EXCEPTION_FLOATING_POINT_ERROR, \
    EXCEPTION_MACHINE_CHECK, \
    EXCEPTION_SIMD_FLOATING_POINT_EXCEPTION, \
    EXCEPTION_RESERVED_2, \
    EXCEPTION_RESERVED_3, \
    EXCEPTION_RESERVED_4, \
    EXCEPTION_RESERVED_5, \
    EXCEPTION_RESERVED_6, \
    EXCEPTION_RESERVED_7, \
    EXCEPTION_RESERVED_8, \
    EXCEPTION_RESERVED_9, \
    EXCEPTION_RESERVED_10, \
    EXCEPTION_RESERVED_11, \
    EXCEPTION_RESERVED_12, \
    EXCEPTION_RESERVED_13
    
    // Align on a word boundary and fill with NOP (0x90)
    .p2align 2, 0x90
    __exception_wrapper_\exceptionNum:
    .type __exception_wrapper_\exceptionNum, @function

        /** --STACK--
        |----------|
        |    SS    |  +16
        |----------|
        |  (ESP)   |  +12
        |----------|
        |  EFLAGS  |  +8
        |----------|
        |    CS    |  +4
        |----------|
        |   EIP    |  <-- ESP
        |----------|
        */

        /* Dummy error code */
        pushl $0
        
        /* Context backup */
        pushl %ebp
        movl %esp, %ebp

        pushl %edi
        pushl %esi
        pushl %eax
        pushl %ebx
        pushl %ecx
        pushl %edx
        pushw %ss
        pushw %ds
        pushw %es
        pushw %fs
        pushw %gs
        
        /* For DWORD alignment */
        subl  $2,%esp

        /* Handler call */
        pushl %esp
        pushl $\exceptionNum
        leal  __exception_handlers, %edi
        call  *\exceptionNum*4(%edi)
                
        /* Point the stack to the saved context */
        addl  $10, %esp
        
        /* Context restore */
        popw  %gs
        popw  %fs
        popw  %es
        popw  %ds
        popw  %ss
        popl  %edx
        popl  %ecx
        popl  %ebx
        popl  %eax
        popl  %esi
        popl  %edi
        popl  %ebp

        /* Error code must not be onto the stack for proper IRET operation */
        addl $4, %esp
        
        iret
.endr

/* Generic wrapper for exception with error code */
.irp exceptionNum, \
    EXCEPTION_DOUBLE_FAULT, \
    EXCEPTION_INVALID_TSS, \
    EXCEPTION_SEGMENT_NOT_PRESENT, \
    EXCEPTION_STACK_SEGMENT_FAULT, \
    EXCEPTION_GENERAL_PROTECTION_FAULT, \
    EXCEPTION_PAGE_FAULT, \
    EXCEPTION_ALIGNEMENT_CHECK
    
    /* Align on a word boundary and fill with NOP (0x90) */
    .p2align 2, 0x90
    __exception_wrapper_\exceptionNum:
    .type __exception_wrapper_\exceptionNum,@function

        /** --STACK--
        |----------|
        |    SS    |  +20
        |----------|
        |  (ESP)   |  +16
        |----------|
        |  EFLAGS  |  +12
        |----------|
        |    CS    |  +8
        |----------|
        |   EIP    |  +4
        |----------|
        |  ERROR#  | <-- ESP
        |----------|
        */

        /* Context backup */
        pushl %ebp
        movl %esp, %ebp

        pushl %edi
        pushl %esi
        pushl %eax
        pushl %ebx
        pushl %ecx
        pushl %edx
        pushw %ss
        pushw %ds
        pushw %es
        pushw %fs
        pushw %gs
        
        /* For DWORD alignment */
        subl  $2,%esp
 
        /* Handler call */
        pushl %esp
        pushl $\exceptionNum
        leal  __exception_handlers, %edi
        call  *\exceptionNum*4(%edi)
        
        /* Point the stack to the saved context */
        addl  $10, %esp

        /* Context restore */
        popw  %gs
        popw  %fs
        popw  %es
        popw  %ds
        popw  %ss
        popl  %edx
        popl  %ecx
        popl  %ebx
        popl  %eax
        popl  %esi
        popl  %edi
        popl  %ebp

        // Error code must not be onto the stack for proper IRET operation
        addl $4, %esp
        
        iret
.endr


.section ".rodata"

/* Align the wrapper vector table on a 32-bit boundary */
.p2align 5, 0x0
__exception_wrappers:
.irp exceptionNum,  \
    EXCEPTION_DIVIDE_ERROR, \
    EXCEPTION_DEBUG, \
    EXCEPTION_NMI_INTERRUPT, \
    EXCEPTION_BREAKPOINT, \
    EXCEPTION_OVERFLOW, \
    EXCEPTION_BOUND_RANGE_EXCEEDED, \
    EXCEPTION_INVALID_OPCODE, \
    EXCEPTION_DEVICE_NOT_AVAILABLE, \
    EXCEPTION_DOUBLE_FAULT, \
    EXCEPTION_COPROCESSOR_SEGMENT_OVERRUN, \
    EXCEPTION_INVALID_TSS, \
    EXCEPTION_SEGMENT_NOT_PRESENT, \
    EXCEPTION_STACK_SEGMENT_FAULT, \
    EXCEPTION_GENERAL_PROTECTION_FAULT, \
    EXCEPTION_PAGE_FAULT, \
    EXCEPTION_RESERVED_1, \
    EXCEPTION_FLOATING_POINT_ERROR, \
    EXCEPTION_ALIGNEMENT_CHECK, \
    EXCEPTION_MACHINE_CHECK, \
    EXCEPTION_SIMD_FLOATING_POINT_EXCEPTION, \
    EXCEPTION_RESERVED_2, \
    EXCEPTION_RESERVED_3, \
    EXCEPTION_RESERVED_4, \
    EXCEPTION_RESERVED_5, \
    EXCEPTION_RESERVED_6, \
    EXCEPTION_RESERVED_7, \
    EXCEPTION_RESERVED_8, \
    EXCEPTION_RESERVED_9, \
    EXCEPTION_RESERVED_10, \
    EXCEPTION_RESERVED_11, \
    EXCEPTION_RESERVED_12, \
    EXCEPTION_RESERVED_13
    .long (__exception_wrapper_\exceptionNum)
.endr

