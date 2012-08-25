#ifndef _HW_IOAPIC_H_
#define _HW_IOAPIC_H_

#include <stdlib.h>
#include <inttypes.h>
#include <kernel/types.h>

namespace Stage3 {
namespace Interrupts {

    typedef struct
    {
        unsigned _reserved      : 24;
        unsigned id             : 4;
        unsigned _reserved2     : 4;
    } __pack ioapicid_t;
    
    typedef struct
    {
        uint8_t version;
        uint8_t _reserved;
        uint8_t max_redir;
        uint8_t _reserved2;
    } __pack ioapicver_t;
    
    typedef struct
    {
        unsigned _reserved      : 24;
        unsigned arb_id         : 4;
        unsigned _reserved2     : 4;
    } __pack ioapicarb_t;
    
    typedef struct _ioredtbl
    {
        uint8_t     vector;
        
        #define IOREDTBL_DELIV_FIXED            0x0
        #define IOREDTBL_DELIV_LOWEST_PRIORITY  0x1
        #define IOREDTBL_DELIV_SMI              0x2
        #define IOREDTBL_DELIV_NMI              0x4
        #define IOREDTBL_DELIV_INIT             0x5
        #define IOREDTBL_DELIV_EXTINT           0x7
        unsigned    delmod      : 3;
        unsigned    destmod     : 1;
        unsigned    delivs      : 1;    // RO
        unsigned    intpol      : 1;
        unsigned    remote_irr  : 1;    // RO
        
        #define IOREDTBL_TRIGGER_LEVEL  1
        #define IOREDTBL_EDGE_LEVEL     0
        unsigned    trigger     : 1;
        unsigned    mask        : 1;
        unsigned    _reserved   : 31;
        unsigned    _reserved2  : 8;
        uint8_t     destination;
        
        _ioredtbl(uint8_t vector, uint8_t delmod, bool destmod,
            bool intpol, bool trigger, bool mask,
            uint8_t destination)
            : vector(vector), delmod(delmod), destmod(destmod), delivs(0),
            intpol(intpol), remote_irr(0), trigger(trigger), mask(mask),
            _reserved(0), _reserved2(0), destination(destination) { }
    } __pack ioredtbl_t;
    
    class IOAPIC
    {
        uint32_t ioMap[24];

        #define IOAPICID            0x00
        #define IOAPICVER           0x01
        #define IOAPICARB           0x02
        #define IOREDTBL(x)         (0x10 + (2*x))
        uint32_t* IOREGSEL;
        uint32_t* IOWIN;
        
        public:
        IOAPIC();
        IOAPIC(uintptr_t base);
        
        void enableIRQ(uint8_t irq);
        void disableIRQ(uint8_t irq);
        uint8_t getIRQfromVector(uint8_t vector);
        void map(uint8_t vector, uint8_t irq, bool level_sensitive,
            bool active_low, uint8_t destination, bool destination_logical);
        private:
        uint32_t    _readReg32(uint8_t offset);
        uint64_t    _readReg64(uint8_t offset);
        void        _writeReg32(uint8_t offset, uint32_t value);
        void        _writeReg64(uint8_t offset, uint64_t value);
    };
    
    extern IOAPIC bspIOAPIC;
}
}

#endif // _HW_IOAPIC_H_
