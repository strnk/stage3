#ifndef _DEVICES_KEYBOARD_H_
#define _DEVICES_KEYBOARD_H_

#include <kernel/devices/8042.hpp>

namespace Stage3 {
namespace Devices {

    class Keyboard : public PS2Device
    {
        enum _kbd_state
        {
            WAIT_INPUT,
            BREAK_CODE,
            EXTENDED_CODE,
            EXTENDED_BREAKED_CODE,
            CMD_SENT
        } state;
        
        public:
            Keyboard(_PS2Controller* controller);
            
            void recvByte(uint8_t data);
            void processScanCode(uint8_t scanCode, bool released, 
                bool extended);
    } ;
    
}
}

#endif // _DEVICES_KEYBOARD_H_
