#include <kernel/devices/keyboard.hpp>
#include <kernel/hw/interrupts.hpp>

#include <cstdio>

using namespace Stage3::Devices;


Stage3::Devices::Keyboard::Keyboard(_PS2Controller* controller)
    : PS2Device(controller), state(WAIT_INPUT)
{
}


void
Stage3::Devices::Keyboard::recvByte(uint8_t data)
{    
    switch (state)
    {
        case WAIT_INPUT:
            if (data == 0xF0)   
                state = BREAK_CODE;
            else if (data == 0xE0)
                state = EXTENDED_CODE;
            else
                processScanCode(data, false, false);
        break;
        
        case EXTENDED_CODE:
            if (data == 0xF0)
                state = EXTENDED_BREAKED_CODE;
            else
            {
                processScanCode(data, false, true);
                state = WAIT_INPUT;
            }
        break;
        
        case EXTENDED_BREAKED_CODE:
            processScanCode(data, true, true);
            state = WAIT_INPUT;
        break;
        
        case BREAK_CODE:
            processScanCode(data, true, false);
            state = WAIT_INPUT;
        break;
        
        default:
            state=WAIT_INPUT;
        break;
    }
}


void 
Stage3::Devices::Keyboard::processScanCode(uint8_t scanCode, bool released,
    bool extended)
{
    printf("KBD recv SC=%x R=%d, X=%d\n", scanCode, released, extended);
}
