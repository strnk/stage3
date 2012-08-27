#ifndef _DEVICES_8042_H_
#define _DEVICES_8042_H_

#include <kernel/types.h>

namespace Stage3 {
namespace Devices {

    #define PS2_DATA_PORT       0x0060
    #define PS2_COMMAND_PORT    0x0064
    
    #define PS2_TIMEOUT         65535
    
    #define PS2_CMD_READ_CONFIG         0x20
    #define PS2_CMD_WRITE_CONFIG        0x60
    #define PS2_CMD_PORT2_DISABLE       0xA7
    #define PS2_CMD_PORT2_ENABLE        0xA8
    #define PS2_CMD_PORT2_TEST          0xA9
    #define PS2_CMD_SELF_TEST           0xAA
    #define PS2_CMD_PORT1_TEST          0xAB
    
    #define PS2_CMD_PORT1_DISABLE       0xAD
    #define PS2_CMD_PORT1_ENABLE        0xAE
    
    #define PS2_CMD_SEND_PORT2_DATA     0xD4
    
    #define PS2_DEVCMD_IDENTIFY         0xF2
    #define PS2_DEVCMD_ENABLE_DATA      0xF4
    #define PS2_DEVCMD_DISABLE_DATA     0xF5
    #define PS2_DEVCMD_ACK              0xFA
    #define PS2_DEVCMD_NACK             0xFC
    #define PS2_DEVCMD_RESET            0xFF
    
    #define PS2_DEVICE_OLD_AT_KBD       0xFFFF
    #define PS2_DEVICE_STANDARD_MOUSE   0x00FF
    #define PS2_DEVICE_SCROLL_MOUSE     0x03FF
    #define PS2_DEVICE_5BTN_MOUSE       0x04FF
    #define PS2_DEVICE_MF2_KBD_TRANS    0xAB41
    #define PS2_DEVICE_MF2_KBD_TRANS2   0xABC1
    #define PS2_DEVICE_MF2_KBD          0xAB83
    
    class PS2Device;
    
    class _PS2Controller
    {
        bool doublePort;
        bool portStatus[2];
        bool deviceStatus[2];
        PS2Device *deviceDrivers[2];
        
        struct _ps2_status_register
        {
            unsigned output_buffer_full : 1;
            unsigned input_buffer_full  : 1;
            unsigned system_flag        : 1;
            unsigned command            : 1; // 0: device / 1: controller
            unsigned unknown            : 1;
            unsigned output_buffer2_full: 1;
            unsigned timeout            : 1;
            unsigned parity_error       : 1;
        } __pack status;
        
        public:
            _PS2Controller();
            
            void init();
            void autoDetect();
            
            void enableIRQ();
            void disableIRQ();
            
            bool deviceReset(uint8_t device);
            
            uint16_t deviceType(uint8_t device);
            
            void irq1();
            void irq12();
            static void _irq1(uint64_t intNum);
            static void _irq12(uint64_t intNum);
        
        protected:
            bool _writeDevice1(uint8_t data);
            bool _writeDevice2(uint8_t data);
            
            bool _readConfiguration(uint8_t* cfg);
            bool _writeConfiguration(uint8_t cfg);
            
            bool _writeCommand(uint8_t cmd);
            uint8_t _readStatus();
            bool _writeData(uint8_t data);
            uint8_t _readData();
            bool _waitForData();
            bool _waitForDataEx();
            bool _waitForWrite();
            
            friend class PS2Device;
    };
    
    class PS2Device
    {
        protected:
            _PS2Controller* controller;
            
        public:
            PS2Device(_PS2Controller* controller)
                : controller(controller) { }
            virtual ~PS2Device() { }
                
            virtual void recvByte(uint8_t data) = 0;
    };
    
    extern _PS2Controller PS2Controller;
}
}
#endif // _DEVICES_8042_H_
