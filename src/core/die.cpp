#include <kernel/die.hpp>
#include <kernel/tty.h>
#include <kernel/tty/videomem.h>

#include <inttypes.h>
#include <stdio.h>

void 
die(const char* message)
{
    int i;
    uint64_t RIP;
    
    asm volatile("movq -24(%%rsp), %0" : "=r"(RIP) :: "memory" );    
    
    tty_clear_screen();
    
    puts(COLOR(ISO6429_BG_RED) LCOLOR(ISO6429_YELLOW));
    tty_move_cursor(0,0); 
    for (i = 0; i < 8; i+=1)
        printf("   PANIC  ");
        
    printf(COLOR(ISO6429_BG_BLACK) LCOLOR(ISO6429_GREY));
    tty_move_cursor(0,1);
    
    printf("Kernel got stuck at 0x%lx\n\n", RIP);
    puts(message);
    
    for (;;)
        asm volatile("hlt");
}
