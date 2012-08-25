#ifndef _DEVICES_KEYBOARD_H_
#define _DEVICES_KEYBOARD_H_

#include <kernel/hw/interrupts.hpp>

void keyboard_interrupt_handler(uint64_t interrupt);

#endif // _DEVICES_KEYBOARD_H_
