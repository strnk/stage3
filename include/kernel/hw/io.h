#ifndef _HW_IO_H_
#define _HW_IO_H_

#include <inttypes.h>

inline void
outb(uint16_t port, uint8_t value);

inline void
outw(uint16_t port, uint16_t value);

inline void
outl(uint16_t port, uint32_t value);

inline uint8_t
inb(uint16_t port);

inline uint16_t
inw(uint16_t port);

inline uint32_t
inl(uint16_t port);

#endif // _HW_IO_H_
