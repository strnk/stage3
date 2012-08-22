#ifndef _HW_SEGMENTATION_H_
#define _HW_SEGMENTATION_H_

#include <kernel/types.h>

// rw bit for code selector
#define GDTE_RW_CODE_READ       1
#define GDTE_RW_CODE_EXEC_ONLY  0

// rw bit for data selector
#define GDTE_RW_DATA_WRITE      1
#define GDTE_RW_DATA_READ_ONLY  0

// dc bit for code selector
#define GDTE_DC_CODE_DPL_CONFORMING     1
#define GDTE_DC_CODE_DPL_STRICT         0

// dc bit for data selector
#define GDTE_DC_DATA_GROWS_DOWN 1
#define GDTE_DC_DATA_GROWS_UP   0

// execute bit
#define GDTE_CODE_SEGMENT   1
#define GDTE_DATA_SEGMENT   0

// type bit
#define GDTE_TYPE_SYSTEM    0
#define GDTE_TYPE_CODE_DATA 1

// present bit
#define GDTE_PRESENT        1
#define GDTE_DISABLED       0

// size bit
#define GDTE_SIZE_16_BITS   0
#define GDTE_SIZE_32_BITS   1

// granularity bit
#define GDTE_GRANULARITY_BYTE   0
#define GDTE_GRANULARITY_PAGE   1

namespace Stage3
{
namespace GDT
{
    struct gdt_entry
    {
        uint16_t size_lo;           // Size[0..15]
        uint16_t base_address_lo;   // Base[0..15]
        uint8_t base_address_mid;   // Base[16..23]
        unsigned accessed : 1;
        unsigned rw : 1;
        unsigned dc : 1;
        unsigned execute : 1;
        unsigned type : 1;
        unsigned dpl : 2;
        unsigned present : 1;
        unsigned size_hi : 4;        // Size[16..19]
        unsigned avl : 1;
        unsigned longmode : 1;
        unsigned op_size : 1;
        unsigned granularity : 1;
        uint8_t base_address_hi;    // Base[24..31]
        
        gdt_entry(uint8_t privilege, bool exec);
    } __packalign(8);

    // Structure of the IDT register
    struct gdt_register {
        uint16_t    size;
        uint32_t    base_address;
    } __pack;

    struct gdt64_register {
        uint16_t    size;
        uint64_t    base_address;
    } __pack;


    void 
    init(void);
}
}

#endif // _HW_SEGMENTATION_H_
