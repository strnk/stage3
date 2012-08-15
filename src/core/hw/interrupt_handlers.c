#include <kernel/tty.h>
#include <kernel/tty/videomem.h>
#include <kernel/hw/interrupts.h>
#include <kernel/hw/interrupt_handlers.h>

#include <stdio.h>

extern uint64_t __exception_wrappers[INTERRUPTS_EXCEPTION_COUNT];
exception_handler_t __exception_handlers[INTERRUPTS_EXCEPTION_COUNT];
interrupt_handler_t __interrupt_handlers[INTERRUPTS_MASKABLE_COUNT];


void
default_exception_handler(int exception, const struct cpu_context *context)
{
    int i;
    tty_clear_screen();
    
    puts(COLOR(ISO6429_BG_RED) LCOLOR(ISO6429_YELLOW));
    tty_move_cursor(0,0); 
    for (i = 0; i < 8; i+=1)
        printf("   PANIC  ");
        
    printf(COLOR(ISO6429_BG_BLACK) LCOLOR(ISO6429_GREY));
    tty_move_cursor(0,1); 
    
    printf("Exception %d at 0x%lx with error %d\nContext from 0x%lx: \n",
        exception, context->rip, context->error, (uint64_t)context);
        
    printf("  RAX=0x%lx    RBX=0x%lx    RCX=0x%lx    RDX=0x%lx\n", 
        context->rax, context->rbx, context->rcx, context->rdx);
    printf("  RSI=0x%lx    RDI=0x%lx\n", 
        context->rsi, context->rdi);
    printf("  RBP=0x%lx\n", 
        context->rbp);
        
    printf("  FS=0x%x    GS=0x%x\n",
        context->fs, context->gs);   
        
    printf("  CS=0x%x\n",
        context->cs);  
    printf("  RIP=0x%x\n", 
        context->rip);
    printf("  RFLAGS=0x%x\n", 
        context->rflags);
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

