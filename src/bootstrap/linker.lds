OUTPUT_FORMAT("elf64-x86-64");

/* The entry point is _start (defined in boot.S) */
ENTRY(_start)

/* The architecture is i386 */
OUTPUT_ARCH("x86-64")

SECTIONS
{
    . = PHYS_KBASE;
    
    .multiboot :
    {  
        *(.multiboot)
    }
    
    __b_load  = .;
    __b_load_phys  = __b_load;
    
    .loader :
    {
        __b_loader = .;
        *(.loader*);
        __e_loader = .;
    }

    . += VIRT_KBASE;
    
    __b_kernel = .;
    __b_kernel_phys  = __b_kernel - VIRT_KBASE;
    
    .text ALIGN(0x1000) : AT(ADDR(.text) - VIRT_KBASE)
    {
        *(.text*)
        *(.gnu.linkonce.t*)
    }

    .data ALIGN(0x1000) : AT(ADDR(.data) - VIRT_KBASE)
    {   
        *(.data) 
        *(.gnu.linkonce.d*)
    }

    .rodata ALIGN(0x1000) : AT(ADDR(.rodata) - VIRT_KBASE)
    {   
        __b_ctors = .;
        *(SORT(.ctors*))
        __e_ctors = .;

        __b_dtors = .;
        *(SORT(.dtors*))
        __e_dtors = .;
        *(.rodata*)
        *(.gnu.linkonce.r*)
    }
    
    __e_load  = .;
    __e_load_phys = __e_load - VIRT_KBASE;

    .bss : AT(ADDR(.bss) - VIRT_KBASE)
    {   
        *(.bss)
        *(COMMON)
        *(.gnu.linkonce.b*)
    }
    
    __e_kernel  = .;
    __e_kernel_phys = __e_kernel - VIRT_KBASE;

    /DISCARD/ : {
            *(.debug_*)
            *(.note*)
            *(.indent)
            *(.comment)
            *(.stab)
            *(.stabstr)
            *(.eh_frame)
    }
}

