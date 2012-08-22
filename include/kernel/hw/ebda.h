#ifndef _HW_EBDA_H_
#define _HW_EBDA_H_

#include <inttypes.h>
#include <kernel/types.h>


#define BDA_EBDA_ADDR   7

void
ebda_init();

phys_addr_t
ebda_search_u32_id(uint32_t id);

phys_addr_t
ebda_search_u64_id(uint64_t id);

#define EBDA_ID32(a, b, c, d) ((d << 24) | (c << 16) | (b << 8) | a)

#define EBDA_ID64(a, b, c, d, e, f, g, h) \
    (uint64_t)(((uint64_t)h << 56) | ((uint64_t)g << 48) | ((uint64_t)f << 40) | ((uint64_t)e << 32) \
    | ((uint64_t)d << 24) | ((uint64_t)c << 16) | ((uint64_t)b << 8) | a)
#endif // _HW_EBDA_H_
