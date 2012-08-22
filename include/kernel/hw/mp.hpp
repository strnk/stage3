#ifndef _HW_MP_H_
#define _HW_MP_H_

#include <kernel/types.h>

struct mp_floating_pointer
{
    char        signature[4];
    uint32_t    phys_ptr;
    uint8_t     length;
    uint8_t     version;
    uint8_t     checksum;
    
    bool isSignatureValid();
    bool isChecksumValid();
} __pack;

#endif // _HW_MP_H_
