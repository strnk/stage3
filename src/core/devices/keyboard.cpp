#include <kernel/devices/keyboard.hpp>
#include <kernel/hw/interrupts.hpp>

#include <cstdio>

void keyboard_interrupt_handler(uint64_t interrupt)
{
    puts("K");
}
