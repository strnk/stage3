#include <kernel/tty.h>
#include <kernel/tty/videomem.h>
#include <kernel/hw/interrupts.h>
#include <kernel/hw/exceptions.h>
#include <kernel/hw/interrupt_handlers.h>

#include <stdio.h>

extern uint64_t __exception_wrappers[INTERRUPTS_EXCEPTION_COUNT];
exception_handler_t __exception_handlers[INTERRUPTS_EXCEPTION_COUNT];
interrupt_handler_t __interrupt_handlers[INTERRUPTS_MASKABLE_COUNT];

const char* strExceptions[INTERRUPTS_EXCEPTION_COUNT] =
    {
        [EXCEPTION_DIVIDE_ERROR]                = "Division by zero",
        [EXCEPTION_DEBUG]                       = "Debug",
        [EXCEPTION_NMI_INTERRUPT]               = "Non-maskable hardware interrupt",
        [EXCEPTION_BREAKPOINT]                  = "Breakpoint",
        [EXCEPTION_OVERFLOW]                    = "Overflow",
        [EXCEPTION_BOUND_RANGE_EXCEEDED]        = "Bound range exceeded",
        [EXCEPTION_INVALID_OPCODE]              = "Invalid opcode",
        [EXCEPTION_DEVICE_NOT_AVAILABLE]        = "FPU disabled",
        [EXCEPTION_DOUBLE_FAULT]                = "Double fault",
        [EXCEPTION_COPROCESSOR_SEGMENT_OVERRUN] = "??? (coprocessor segment overrun)",
        [EXCEPTION_INVALID_TSS]                 = "Invalid TSS",
        [EXCEPTION_SEGMENT_NOT_PRESENT]         = "Invalid segment",
        [EXCEPTION_STACK_SEGMENT_FAULT]         = "Invalid stack segment",
        [EXCEPTION_GENERAL_PROTECTION_FAULT]    = "General protection fault",
        [EXCEPTION_PAGE_FAULT]                  = "Page fault",
        [EXCEPTION_RESERVED_1]                  = "??? reserved #1",
        [EXCEPTION_FLOATING_POINT_ERROR]        = "Floating point error",
        [EXCEPTION_ALIGNEMENT_CHECK]            = "Bad alignment",
        [EXCEPTION_MACHINE_CHECK]               = "Machine check",
        [EXCEPTION_SIMD_FLOATING_POINT_EXCEPTION]= "SIMD floating point exception"
    };
    
void
default_exception_handler(uint64_t exception, const struct cpu_context *context)
{
    int i;
    
    tty_clear_screen();
    
    puts(COLOR(ISO6429_BG_RED) LCOLOR(ISO6429_YELLOW));
    tty_move_cursor(0,0); 
    for (i = 0; i < 8; i+=1)
        printf("   PANIC  ");
        
    printf(COLOR(ISO6429_BG_BLACK) LCOLOR(ISO6429_GREY));
    tty_move_cursor(0,1); 
    
    printf("Exception %ld : %s\nError #: %ld\n",
        exception, strExceptions[exception], context->error);
        
    printf("  RAX=0x%lx    RBX=0x%lx    RCX=0x%lx\n", 
        context->rax, context->rbx, context->rcx);
    printf("  RDX=0x%lx    RSI=0x%lx    RDI=0x%lx\n", 
        context->rdx, context->rsi, context->rdi);
    printf("  RBP=0x%lx\n", 
        context->rbp);
        
    printf("  FS=0x%x    GS=0x%x    CS=0x%x\n\n",
        context->fs, context->gs, context->cs);  
    printf("  RIP=0x%lx\n", 
        context->rip);
    printf("  RFLAGS=0x%lx\n\n", 
        context->rflags);
        
    switch (exception)
    {
        case EXCEPTION_PAGE_FAULT:
        {
            uint64_t CR2;
            
            /* CR2 contain the faulty page */
            asm volatile(
                "movq %%cr2, %0;"
                : "=r"(CR2)
                ::
                );
                
            if (context->error & EXCEPTION_PF_WRITE_ACCESS)
                printf("write");
            else
                printf("read");
                
            printf(" access to a ");
            
            if (context->error & EXCEPTION_PF_PROTECTION_VIOLATION)
                printf("protected");
            else
                printf("non-present");
                
            printf(" page in ");
            
            if (context->error & EXCEPTION_PF_USER_MODE)
                printf("user");
            else
                printf("supervisor");
                
            printf(" mode.\n");
            printf("At: %lx\n", CR2);          
        }
        break;
    }
    for (;;)
        asm volatile("hlt" : : );
}


void
interrupt_init(void)
{
    int i;
    
    // Init the exception handlers
    exception_init();
    
    // Assign wrappers to each exception
    for (i = 0; i < INTERRUPTS_EXCEPTION_COUNT; i++)
    {
        idt_set_handler(INTERRUPTS_EXCEPTION_BASE + i, 
            __exception_wrappers[i], 0);
    }
}

void
exception_init(void)
{
    int i;
    
    for (i = 0; i < INTERRUPTS_EXCEPTION_COUNT; i++)
    {
        exception_set_handler(i, default_exception_handler);
    }
}


void
exception_set_handler(int exception, exception_handler_t handler)
{
    __exception_handlers[exception] = handler;
}

