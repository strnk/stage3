#include <multiboot.h>


#define EXT_C(sym)                      sym
#define STACK_SIZE                      0x4000

#ifdef __ELF__
# ifdef __x86_64__
#  define MULTIBOOT_HEADER_FLAGS        MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO | MULTIBOOT_AOUT_KLUDGE
#  define SECTION_TYPE        "a", @progbits
#  define BOOT_AOUT_KLUDGE
# else
#  define MULTIBOOT_HEADER_FLAGS         MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO
#  define SECTION_TYPE         "a", @progbits
#  undef  BOOT_AOUT_KLUDGE
# endif
#else
# define MULTIBOOT_HEADER_FLAGS         MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO | MULTIBOOT_AOUT_KLUDGE
# define SECTION_TYPE         "d"
# define BOOT_AOUT_KLUDGE
#endif

    .global  start, _start
    
    
    .section .multiboot, SECTION_TYPE

multiboot_header:
    .long   MULTIBOOT_HEADER_MAGIC
    .long   MULTIBOOT_HEADER_FLAGS
    .long   -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
#ifdef BOOT_AOUT_KLUDGE
    /* header_addr */
    .long   multiboot_header
    /* load_addr */
    .long   __b_load_phys
    /* load_end_addr */
    .long   __e_load_phys
    /* bss_end_addr */
    .long   __e_kernel_phys
    /* entry_addr */
    .long   _start
#endif /* ! __ELF__ */

    .code32
    .section .loader32, SECTION_TYPE
    
start:
_start:
    // Save the multiboot data
    movl    $0x101000, %esp
    movl    %esp, %ebp
    
    pushl   $0
    pushl   %eax
    pushl   $0
    pushl   %ebx
    
/*
    Temporary paging :
    0x101000 - PML4
    0x102000 - PDPT 0
    0x103000 - PD 0.0
    0x104000 - PT 0.0.0       0x0000 00000000 - 0x0000 001FFFFF
    0x105000 - PT 0.0.1       0x0000 00200000 - 0x0000 003FFFFF
    0x106000 - PDPT 1ff
    0x107000 - PD 1ff.1fe
    0x108000 - PT 1ff.1fe.1   0xFFFF 80200000 - 0xFFFF 803FFFFF
    0x109000 - PT 1ff.1fe.2   0xFFFF 80400000 - 0xFFFF 805FFFFF
    0x10A000 - PT 1ff.1fe.3   0xFFFF 80600000 - 0xFFFF 807FFFFF
    0x10B000 - PT 1ff.1fe.4   0xFFFF 80800000 - 0xFFFF 809FFFFF
    0x10C000 - PT 1ff.1fe.5   0xFFFF 80A00000 - 0xFFFF 80BFFFFF
    0x10D000 - PT 1ff.1fe.6   0xFFFF 80C00000 - 0xFFFF 80DFFFFF
    0x10E000 - PT 1ff.1fe.7   0xFFFF 80E00000 - 0xFFFF 80FFFFFF
 */
setupTempPML4:
    movl    $0x101000, %edi    
    xor     %eax, %eax
    movl    $0x9000, %ecx
    rep     stosl           // clear the paging data address space
    
    movl    $0x101000, %edi
    movl    $0x102003, (%edi)         // PML4[0] -> PDPT 0,     RW
    movl    $0x1ff, %eax
    movl    $0x106003, (%edi, %eax, 8)    // PML4[1ff] -> PDPT 1ff, RW
    
setupTempPDPT:
    movl    $0x102000, %edi
    movl    $0x103003, (%edi)   // PDPT0[0] -> PD0.0,   RW
    
    movl    $0x106000, %edi
    movl    $0x1fe, %eax
    movl    $0x107003, (%edi, %eax, 8)   // PDPT 1ff[1fe] -> PD 1ff.1fe
    
setupTempPDT:
    movl    $0x103000, %edi
    movl    $0x104003, (%edi)    // PD0.0[0] -> PT0.0.0, RW
    movl    $0x105003, 8(%edi)   // PD0.0[1] -> PT0.0.1, RW
    
    
    movl    $0x107000, %edi
    movl    $0x108003, 8(%edi)    // PD1ff.1fe[1] -> PT1ff.1fe.1, RW
    movl    $0x109003, 16(%edi)   // PD1ff.1fe[2] -> PT1ff.1fe.2, RW
    movl    $0x10A003, 24(%edi)   // PD1ff.1fe[3] -> PT1ff.1fe.3, RW
    movl    $0x10B003, 32(%edi)   // PD1ff.1fe[4] -> PT1ff.1fe.4, RW
    movl    $0x10C003, 40(%edi)   // PD1ff.1fe[5] -> PT1ff.1fe.5, RW
    movl    $0x10D003, 48(%edi)   // PD1ff.1fe[6] -> PT1ff.1fe.6, RW
    movl    $0x10E003, 56(%edi)   // PD1ff.1fe[7] -> PT1ff.1fe.7, RW
    jmp     setupIdentityMap
    
setupTempPT:
    orl     $3, %ebx
    movl    $512, %ecx
    
fillPT:
    movl    %ebx, (%edi)
    addl    $8, %edi
    addl    $0x1000, %ebx
    loop    fillPT
    ret
    
setupIdentityMap:
    movl    $0x104000, %edi
    movl    $0x00000000, %ebx
    call    setupTempPT
    
    movl    $0x105000, %edi
    movl    $0x00200000, %ebx
    call    setupTempPT
    
    movl    $0x108000, %edi
    movl    $0x00200000, %ebx
    call    setupTempPT
    
    movl    $0x109000, %edi
    movl    $0x00400000, %ebx
    call    setupTempPT    
    
    movl    $0x10A000, %edi
    movl    $0x00600000, %ebx
    call    setupTempPT   
     
    movl    $0x10B000, %edi
    movl    $0x00800000, %ebx
    call    setupTempPT
        
    movl    $0x10C000, %edi
    movl    $0x00A00000, %ebx
    call    setupTempPT    
    
    movl    $0x10D000, %edi
    movl    $0x00C00000, %ebx
    call    setupTempPT   
     
    movl    $0x10E000, %edi
    movl    $0x00E00000, %ebx
    call    setupTempPT    
    
enablePAE:
    movl    %cr4, %eax
    bts     $5, %eax
    movl    %eax, %cr4
    
    movl    $0x101000, %eax
    movl    %eax, %cr3          // PML4 address
    
enableCompatibilityMode:
    movl    $0xC0000080, %ecx
    rdmsr                       // read EFER MSR
    bts     $8, %eax
    wrmsr
    
enablePaging:
    movl    %cr0, %eax
    bts     $31, %eax
    movl    %eax, %cr0
    
setupTempGDT:
    lgdt    TempGDT.ptr
    ljmp    $8, $_64bitEntry    // jump into code segment
    
    
/**
    64-bit long mode
 **/
    .code64
    .section .loader64, SECTION_TYPE
    
_64bitEntry:
1:
    cli
    movw    $16, %ax            // set the data segments
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %fs
    movw    %ax, %gs
    jmp     multiboot_entry
    
multiboot_entry:
    popq    %rsi    // saved multiboot magic -- 1st main() arg
    popq    %rdi    // saved multiboot data ptr -- 2nd main() arg
    
    /* Initialize the stack pointer. */
    movq    $(0x110000 + STACK_SIZE), %rsp

    // clear EFLAG
    pushq   $0
    popf
    
    movq    $EXT_C(main), %rax
    call    *%rax

    /* Halt. */
    loop:   hlt
    jmp     loop

TempGDT:
    .word 0                 // limit 0-15
    .word 0                 // base 0-15
    .byte 0                 // base 16-23
    .byte 0                 // access
    .byte 0                 // granularity
    .byte 0                 // base 23-31
    
    // Code segment
    .word 0                 // limit 0-31
    .word 0                 // base 0-31
    .byte 0                 // base 32-47
    .byte 0b10011000        // access
    .byte 0b00100000        // granularity
    .byte 0                 // base 48-64
    
    // Data segment
    .word 0                 // limit 0-31
    .word 0                 // base 0-31
    .byte 0                 // base 32-47
    .byte 0b10010000        // access
    .byte 0b00000000        // granularity
    .byte 0                 // base 48-64
    
TempGDT.ptr:
    .word . - TempGDT - 1
    .quad TempGDT
    
