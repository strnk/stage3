#ifndef _HW_INTERRUPTS_H_
#define _HW_INTERRUPTS_H_

#ifndef ASM_SOURCE
# include <kernel/types.h>
# include <stddef.h>
#endif

/* IDT entry type */ 
#define IDTE_TYPE_TSS_AVAILABLE         0x09
#define IDTE_TYPE_TSS_BUSY              0x0B
#define IDTE_TYPE_CALL_GATE             0x0C
#define IDTE_TYPE_INTERRUPT_GATE        0x0E
#define IDTE_TYPE_TRAP_GATE             0x0F 

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

#ifndef ASM_SOURCE

namespace Stage3 { 
namespace Interrupts {
    namespace IDT
    {
        // Structure of an IDT entry
        struct idt_entry {
            uint16_t   offset_lo;  // Offset[0..15]
            uint16_t   selector;   // Code segment in GDT
            
            int         ist : 3;
            int         zero1 : 5;
            
            int         type : 4;
            int         zero2 : 1;
            int         dpl : 2;
            int         present : 1;
            
            uint16_t   offset_mid;  // Offset[16..31]
            uint32_t   offset_hi;   // Offset[32..63]
            uint32_t   reserved;
            
            
            void init(uint16_t segment, uint8_t type);
            void set_handler(virt_addr_t handler, uint8_t dpl);
            void disable();
        } __packalign(16);

        // Structure of the IDT register
        struct idt_register {
            uint16_t    size;
            uint64_t    base_address;
            
            idt_register()
                : size(0), base_address(0) {};
            idt_register(uint64_t base_address, size_t size) 
                : size((uint16_t)size - 1), base_address(base_address) {};
            void load();
        } __packalign(16);

        void
        init(void);

        void
        define_handler(uint8_t vector, virt_addr_t handler, uint8_t dpl);
    }

    typedef void 
    (*interrupt_handler_t)(uint64_t interrupt);
    
    void
    define_handler(uint8_t vector, interrupt_handler_t handler);
    
    void
    init(void);
    
    void
    default_handler(uint64_t intNum);
    
    class Manager
    {        
        public:
        typedef enum _int_bus_type_t
        {
            BUS_ISA = 0,
            BUS_PCI
        } bus_type_t;
        
        virtual ~Manager() { }
        virtual uint8_t getType() { return 0; }
        
        virtual void init() = 0;
        virtual void shutdown() = 0;
        
        virtual void enable(bus_type_t bus, uint8_t irq) = 0;
        virtual void disable(bus_type_t bus, uint8_t irq) = 0;
        
        virtual void map(bus_type_t bus, uint8_t irq, uint8_t vector) = 0;
        virtual void eoi(uint64_t vector) = 0;
    };
    
    extern Manager *GlobalManager;
}
}

extern "C"
void __int_manager_eoi(uint64_t vector);
#endif // ASM_SOURCE

#endif // _HW_INTERRUPTS_H_
