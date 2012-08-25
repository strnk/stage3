#ifndef _HW_MP_H_
#define _HW_MP_H_

#include <kernel/types.h>

namespace Stage3 {
namespace MP {

    struct table_floating_pointer
    {
        char        signature[4];
        uint32_t    phys_ptr;
        uint8_t     length;
        uint8_t     version;
        uint8_t     checksum;
        uint8_t     feature[5];
        
        bool isSignatureValid();
        bool isChecksumValid();
    } __pack;
    
    struct config_entry_processor
    {
        uint8_t     entry_type;
        uint8_t     lapic_id;
        uint8_t     lapic_version;
        uint8_t     cpu_flags;
        uint32_t    cpu_signature;
        uint32_t    cpu_features;
        uint64_t    reserved;
    } __pack;
    
    #define MP_CFG_BUS_ISA  0
    #define MP_CFG_BUS_PCI  1
    
    struct config_entry_bus
    {
        uint8_t     entry_type;
        uint8_t     bus_id;
        char        bus_type[6];
        
        bool isBus(const char* bus) {
            return bus_type[0] == bus[0]
                && bus_type[1] == bus[1]
                && bus_type[2] == bus[2]
                && bus_type[3] == bus[3]
                && bus_type[4] == bus[4]
                && bus_type[5] == bus[5];
        }
    } __pack;
    
    struct config_entry_ioapic
    {
        uint8_t     entry_type;
        uint8_t     ioapic_id;
        uint8_t     ioapic_version;
        unsigned    enabled : 1;
        unsigned    _reserved : 7;
        uint32_t    ioapic_address;
        
        bool isEnabled() { return enabled & 1; }
    } __pack;
    
    struct config_entry_iointerrupt
    {
        uint8_t     entry_type;
        
        #define     MP_CFG_INT_INT      0
        #define     MP_CFG_INT_NMI      1
        #define     MP_CFG_INT_SMI      2
        #define     MP_CFG_INT_EXTINT   3
        uint8_t     int_type;
        
        unsigned    po  : 2;        // Polarity
        unsigned    el  : 2;        // Trigger mode
        unsigned    _reserved : 12;
        uint8_t     source_id;      // bus ID
        uint8_t     source_irq;
        uint8_t     dest_id;        // IOAPIC ID
        uint8_t     dest_irq;
    } __pack;
    
    struct config_entry_linterrupt
    {
        uint8_t     entry_type;
        uint8_t     int_type;
        unsigned    po  : 2;        // Polarity
        unsigned    el  : 2;        // Trigger mode
        unsigned    _reserved : 12;
        uint8_t     source_id;      // bus ID
        uint8_t     source_irq;
        uint8_t     dest_id;        // LAPIC ID
        uint8_t     dest_irq;       // LAPIC LINTIN#
    } __pack;
    
    typedef enum _config_entry_type
    {
        PROCESSOR       = 0,
        BUS,
        IOAPIC,
        IOINTERRUPT,
        LINTERRUPT,
        
        // Extended config.
        SYSTEM_ADDR_MAPPING = 128,
        BUS_HIERARCHY,
        BUS_ADDR_MODIFIER
    } config_entry_type_t;
    
    struct configuration_table
    {
        char        signature[4];
        uint16_t    length;
        uint8_t     version;
        uint8_t     checksum;
        
        char        oem_id[8];
        char        prod_id[12];
        
        uint32_t    oem_table_ptr;
        uint16_t    oem_table_size;
        uint16_t    entry_count;
        
        uint32_t    local_apic_ptr;
        
        uint16_t    xtable_length;
        uint16_t    xtable_checksum;
                
        class iterator
        {
            public:
            uintptr_t p;
            uint16_t entry_count_left;
            uint16_t xtable_length_left;
            
            public:
            iterator();
            
            iterator(uintptr_t ptr, uint16_t entry_count, 
                uint16_t xtable_length);
                
            iterator(const iterator& it);
            
            iterator 
            operator++(int);
            
            iterator& 
            operator++();
            
            bool 
            operator==(const iterator& rhs);
            
            bool 
            operator!=(const iterator& rhs);
            
            config_entry_type_t
            type();
            
            config_entry_processor&
            as_processor();
            
            config_entry_bus&
            as_bus();
            
            config_entry_ioapic&
            as_ioapic();
            
            config_entry_iointerrupt&
            as_iointerrupt();
            
            config_entry_linterrupt&
            as_linterrupt();
        };
        
        
        bool        isSignatureValid();
        iterator    begin();
        iterator    end();
    } __pack;
    
    void init();
    bool hasIOAPIC();
    uint8_t getFirstIOAPIC();
    uint32_t getIOAPICaddr(uint8_t id);
    uint8_t busType(uint8_t busId);
    configuration_table* configTablePtr();
}
}
#endif // _HW_MP_H_
