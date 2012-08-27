#include <kernel/hw/exceptions.hpp>
#include <kernel/hw/interrupts.hpp>
#include <kernel/tty/videomem.h>
#include <kernel/tty.h>
#include <stdio.h>

using namespace Stage3::Exceptions;

extern uint64_t __exception_wrappers[INTERRUPTS_EXCEPTION_COUNT];
handler_t __exception_handlers[INTERRUPTS_EXCEPTION_COUNT];

const char* strExceptions[INTERRUPTS_EXCEPTION_COUNT] =
    {
        "Division by zero",
        "Debug",
        "Non-maskable hardware interrupt",
        "Breakpoint",
        "Overflow",
        "Bound range exceeded",
        "Invalid opcode",
        "FPU disabled",
        "Double fault",
        "??? (coprocessor segment overrun)",
        "Invalid TSS",
        "Invalid segment",
        "Invalid stack segment",
        "General protection fault",
        "Page fault",
        "??? reserved #1",
        "Floating point error",
        "Bad alignment",
        "Machine check",
        "SIMD floating point exception"
    };
    

void
Stage3::Exceptions::init(void)
{
    int i;    
    for (i = 0; i < INTERRUPTS_EXCEPTION_COUNT; i++)
    {
        Stage3::Interrupts::IDT::define_handler(INTERRUPTS_EXCEPTION_BASE + i, 
            __exception_wrappers[i], 0);
        Stage3::Exceptions::define_handler(i, 
            Stage3::Exceptions::default_handler);
    }
}

void
Stage3::Exceptions::define_handler(int exception, handler_t handler)
{
    __exception_handlers[exception] = handler;
}
    
void
Stage3::Exceptions::default_handler(uint64_t exception, const struct cpu_context *context)
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

