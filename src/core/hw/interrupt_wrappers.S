#include <kernel/hw/interrupts.hpp>
#include <kernel/hw/exceptions.hpp>

.file "interrupt_wrappers.S"

.global __exception_wrappers
.global __interrupt_wrappers
.extern __exception_handlers
.extern __interrupt_handlers
.extern __int_manager_eoi

.text
.code64

/* Generic wrapper for interrupts */
.irp intNum,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  \
             10, 11, 12, 13, 14, 15
    
    // Align on a dword boundary and fill with NOP (0x90)
    .p2align 4, 0x90
    __interrupt_wrapper_\intNum:
    .type __interrupt_wrapper_\intNum, @function

        /**    --STACK--
        |------------------|
        |          |  SS   |  +32
        |------------------|
        |       (rsp)      |  +24
        |------------------|
        |     RFLAGS       |  +16
        |------------------|
        |          |   CS  |  +8
        |------------------|
        |       RIP        |  <-- rsp
        |------------------|
        */

        /* Context backup */
        pushq %rbp
        movq %rsp, %rbp

        pushq %rdi
        pushq %rsi
        pushq %rax
        pushq %rbx
        pushq %rcx
        pushq %rdx
        pushw %fs
        pushw %gs
        
        subq $4, %rsp
        
        /* Handler call */
        movq %rsp, %rsi
        movq $\intNum, %rdi
        
        leaq  __interrupt_handlers, %r8
        call  *\intNum*8(%r8)
        
        movq $\intNum, %rdi
        movq $__int_manager_eoi, %r8
        call *%r8
                
        addq $4, %rsp
        
        /* Context restore */
        popw  %gs
        popw  %fs
        popq  %rdx
        popq  %rcx
        popq  %rbx
        popq  %rax
        popq  %rsi
        popq  %rdi
        movq  %rbp, %rsp
        popq  %rbp
        
        iretq
.endr

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
    
    // Align on a dword boundary and fill with NOP (0x90)
    .p2align 4, 0x90
    __exception_wrapper_\exceptionNum:
    .type __exception_wrapper_\exceptionNum, @function

        /**    --STACK--
        |------------------|
        |          |  SS   |  +32
        |------------------|
        |       (rsp)      |  +24
        |------------------|
        |     RFLAGS       |  +16
        |------------------|
        |          |   CS  |  +8
        |------------------|
        |       RIP        |  <-- rsp
        |------------------|
        */

        /* Dummy error code */
        pushq $0
        
        /* Context backup */
        pushq %rbp
        movq %rsp, %rbp

        pushq %rdi
        pushq %rsi
        pushq %rax
        pushq %rbx
        pushq %rcx
        pushq %rdx
        pushw %fs
        pushw %gs
        
        subq $4, %rsp
        
        /* Handler call */
        movq %rsp, %rsi
        movq $\exceptionNum, %rdi
        
        leaq  __exception_handlers, %r8
        call  *\exceptionNum*8(%r8)
                
        /* Point the stack to the saved context */
        addq  $4, %rsp
        
        /* Context restore */
        popw  %gs
        popw  %fs
        popq  %rdx
        popq  %rcx
        popq  %rbx
        popq  %rax
        popq  %rsi
        popq  %rdi
        popq  %rbp

        /* Error code must not be onto the stack for proper IRET operation */
        addq $8, %rsp
        
        iretq
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
    
    /* Align on a dword boundary and fill with NOP (0x90) */
    .p2align 4, 0x90
    __exception_wrapper_\exceptionNum:
    .type __exception_wrapper_\exceptionNum,@function

        /**    --STACK--
        |------------------|
        |          |  SS   |  +40
        |------------------|
        |       (rsp)      |  +32
        |------------------|
        |     RFLAGS       |  +24
        |------------------|
        |          |   CS  |  +16
        |------------------|
        |       RIP        |  +8
        |------------------|
        |      ERROR       |  <-- rsp
        |------------------|
        */

        /* Context backup */
        pushq %rbp
        movq %rsp, %rbp

        pushq %rdi
        pushq %rsi
        pushq %rax
        pushq %rbx
        pushq %rcx
        pushq %rdx
        pushw %fs
        pushw %gs
        
        /* For DWORD alignment */
        subq $4, %rsp
 
        /* Handler call */
        movq %rsp, %rsi
        movq $\exceptionNum, %rdi
        
        leaq  __exception_handlers, %r8
        call  *\exceptionNum*8(%r8)
        
        /* Point the stack to the saved context */
        addq  $4, %rsp

        /* Context restore */
        popw  %gs
        popw  %fs
        popq  %rdx
        popq  %rcx
        popq  %rbx
        popq  %rax
        popq  %rsi
        popq  %rdi
        popq  %rbp
        
        // Error code must not be onto the stack for proper IRET operation
        addq $8, %rsp
        
        iretq
.endr


.section ".rodata"

/* Align the interrupt wrapper vector table on a 32-bit boundary */
.p2align 5, 0x0
__interrupt_wrappers:
.irp intNum,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  \
             10, 11, 12, 13, 14, 15
    .quad (__interrupt_wrapper_\intNum)
.endr

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
    .quad (__exception_wrapper_\exceptionNum)
.endr

