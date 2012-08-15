#ifndef _KERNEL_TYPES_H_
#define _KERNEL_TYPES_H_

#include <inttypes.h>

typedef uint64_t phys_addr_t;
typedef uint64_t virt_addr_t;

#define __pack __attribute__((packed))
#define __packalign(n) __attribute__((packed, aligned(n)))
#define __align(n) __attribute__((aligned(n)))
#endif // _KERNEL_TYPES_H_
