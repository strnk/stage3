#include <kernel/devices/8042.hpp>
#include <kernel/devices/keyboard.hpp>
#include <kernel/hw/io.h>

#include <cstdio>

Stage3::Devices::_PS2Controller Stage3::Devices::PS2Controller;

using namespace Stage3::Devices;

Stage3::Devices::_PS2Controller::_PS2Controller()
    : doublePort(true), portStatus({false, false}),
    deviceStatus({false, false}), deviceDrivers({NULL, NULL})
{

}

void
Stage3::Devices::_PS2Controller::init()
{
    uint8_t cfgByte;
    
    // Disable devices
    _writeCommand(PS2_CMD_PORT1_DISABLE);
    _writeCommand(PS2_CMD_PORT2_DISABLE);
    
    // output buffer flush
    _readData();
    
    // config byte set-up
    _readConfiguration(&cfgByte);
    
    if (!(cfgByte & 0x20))
        doublePort = false;
        
    cfgByte &= 0xBC;
    _writeConfiguration(cfgByte);
    
    // Self-test
    _writeCommand(PS2_CMD_SELF_TEST);
    if (!_waitForData() || _readData() != 0x55)
    {
        puts("PS/2: self test failed");
        return;
    }
    
    // Second double-port check if needed
    if (doublePort)
    {
        _writeCommand(PS2_CMD_PORT2_ENABLE);
        
        _readConfiguration(&cfgByte);
        
        if (cfgByte & 0x20)
            doublePort = false;
        else
            _writeCommand(PS2_CMD_PORT2_DISABLE);
    }
    
    // Check interfaces
    _writeCommand(PS2_CMD_PORT1_TEST);
    if (!_waitForData() || _readData() != 0x00)
    {
        portStatus[0] = false;
        puts("PS/2: port 1 test failed");
    }
    portStatus[0] = true;
        
    if (doublePort)
    {
        _writeCommand(PS2_CMD_PORT2_TEST);
        if (!_waitForData() || _readData() != 0x00)
        {
            portStatus[1] = false;
            puts("PS/2: port 2 test failed");
        }
        portStatus[1] = true;
    }
    
    // Enable ports
    if (portStatus[0])
    {
        _writeCommand(PS2_CMD_PORT1_ENABLE);
        deviceReset(0);
    }
        
    if (portStatus[1])
    {
        _writeCommand(PS2_CMD_PORT2_ENABLE);
        deviceReset(1);
    }
    
    while (_waitForDataEx())
        _readData();
}

void
Stage3::Devices::_PS2Controller::autoDetect()
{
    for (unsigned i = 0; i <= 1; i++)
    {
        uint16_t deviceId;
        
        printf("PS/2: auto-detecting device %d ... ", i);
        
        deviceId = deviceType(i);
        
        switch (deviceId)
        {
            case PS2_DEVICE_OLD_AT_KBD:
                puts("AT keyboard (no driver)");
            break;
            
            case PS2_DEVICE_STANDARD_MOUSE:
                puts("standard mouse (no driver)");
            break;
            
            case PS2_DEVICE_SCROLL_MOUSE:
                puts("scroll mouse (no driver)");
            break;
            
            case PS2_DEVICE_5BTN_MOUSE:
                puts("5 buttons mouse (no driver)");
            break;
            
            case PS2_DEVICE_MF2_KBD_TRANS:
            case PS2_DEVICE_MF2_KBD_TRANS2:
            case PS2_DEVICE_MF2_KBD:
                puts("standard MF2 keyboard");
                deviceDrivers[i] = new Stage3::Devices::Keyboard(this);
            break;
        }
    }
}

void
Stage3::Devices::_PS2Controller::enableIRQ()
{
    uint8_t cfgByte;
    
    if (!_readConfiguration(&cfgByte))
        return;
        
    cfgByte |= 0x03;
    
    _writeConfiguration(cfgByte);
}

void
Stage3::Devices::_PS2Controller::disableIRQ()
{
    uint8_t cfgByte;
    
    if (!_readConfiguration(&cfgByte))
        return;
        
    cfgByte &= 0xFC;
    
    _writeConfiguration(cfgByte);
}


bool 
Stage3::Devices::_PS2Controller::deviceReset(uint8_t device)
{
    bool cmdSend;
    
    if (device == 0)
        cmdSend = _writeDevice1(0xFF);
    else if (device == 1)
        cmdSend = _writeDevice2(0xFF);
    else
    {
        printf("PS/2 device %d doesn't exists\n", device);
        return false;
    }

    deviceStatus[device] = (cmdSend && _waitForData() && _readData() == 0xFA
        && _readData() == 0xAA);
        
    printf("PS/2: device %d %s\n", device, deviceStatus[device]?"up":"down");
    return deviceStatus[device];
}

uint16_t
Stage3::Devices::_PS2Controller::deviceType(uint8_t device)
{
    uint8_t id[2] = { 0xFF, 0xFF };
    unsigned i = 0;
    
    _writeCommand(PS2_CMD_PORT2_DISABLE);
    _writeCommand(PS2_CMD_PORT1_DISABLE);
    
    if (device == 0)
    {
        _writeCommand(PS2_CMD_PORT1_ENABLE);
        _writeDevice1(PS2_DEVCMD_DISABLE_DATA);
        while (_waitForDataEx())
            _readData();
        
        _writeDevice1(PS2_DEVCMD_IDENTIFY);
        _waitForData();
        _readData();
        
        while (_waitForDataEx())
            id[i++] = _readData();
            
        _writeDevice1(PS2_DEVCMD_ENABLE_DATA);
        _waitForData();
        _readData();
        _writeCommand(PS2_CMD_PORT2_ENABLE);
    }
    else
    {
        _writeCommand(PS2_CMD_PORT2_ENABLE);
        _writeDevice2(PS2_DEVCMD_DISABLE_DATA);
        while (_waitForDataEx())
            _readData();
        
        _writeDevice2(PS2_DEVCMD_IDENTIFY);
        _waitForData();
        _readData();
        
        while (_waitForDataEx())
            id[i++] = _readData();
            
        _writeDevice2(PS2_DEVCMD_ENABLE_DATA);
        _waitForData();
        _readData();
        _writeCommand(PS2_CMD_PORT1_ENABLE);
    }
    
    return ((uint16_t)id[0] << 8) | id[1];
}

bool 
Stage3::Devices::_PS2Controller::_writeDevice1(uint8_t data)
{
    return _waitForWrite() && _writeData(data);
}

bool 
Stage3::Devices::_PS2Controller::_writeDevice2(uint8_t data)
{
    if (!portStatus[1])
        return false;
        
    if (!_writeCommand(PS2_CMD_SEND_PORT2_DATA))
    {
        puts("PS/2: data redirection to port 2 failed");
        return false;
    }
    
    return _waitForWrite() && _writeData(data);
}

bool
Stage3::Devices::_PS2Controller::_readConfiguration(uint8_t* cfg)
{
    if (!_writeCommand(PS2_CMD_READ_CONFIG))
        return false;
        
    if (!_waitForData())
        return false;
        
    *cfg = _readData();
    return true;
}

bool
Stage3::Devices::_PS2Controller::_writeConfiguration(uint8_t cfg)
{
    if (!_writeCommand(PS2_CMD_WRITE_CONFIG))
        return false;
        
    return _writeData(cfg);
}

bool 
Stage3::Devices::_PS2Controller::_writeCommand(uint8_t cmd)
{
    outb(PS2_COMMAND_PORT, cmd);
    
    return _waitForWrite();
}

uint8_t 
Stage3::Devices::_PS2Controller::_readStatus()
{
    *(uint8_t*)&status = inb(PS2_COMMAND_PORT);
    return *(uint8_t*)&status;
}

bool 
Stage3::Devices::_PS2Controller::_writeData(uint8_t data)
{
    outb(PS2_DATA_PORT, data);    
    
    return _waitForWrite();
}

uint8_t 
Stage3::Devices::_PS2Controller::_readData()
{
    uint8_t b = inb(PS2_DATA_PORT);
    
    //printf("_read: %x\n", b);
    
    return b;
}

bool
Stage3::Devices::_PS2Controller::_waitForData()
{
    _readStatus();
    for (int i = 0; i < PS2_TIMEOUT && !status.output_buffer_full; i++)
        _readStatus();
        
    if (status.output_buffer_full)
        return true;
        
    printf("PS/2: waiting for data failed\n");
    return false;
}

bool
Stage3::Devices::_PS2Controller::_waitForDataEx()
{
    _readStatus();
    for (int i = 0; i < PS2_TIMEOUT && !status.output_buffer_full; i++)
        _readStatus();
        
    return (status.output_buffer_full);
}

bool
Stage3::Devices::_PS2Controller::_waitForWrite()
{
    _readStatus();
    for (int i = 0; i < PS2_TIMEOUT && status.input_buffer_full; i++)
        _readStatus();
        
    if (!status.input_buffer_full)
        return true;
        
    printf("PS/2: writing failed\n");
    return false;
}

void 
Stage3::Devices::_PS2Controller::irq1()
{
    uint8_t data = _readData();
    
    if (deviceDrivers[0] != NULL)
        deviceDrivers[0]->recvByte(data);
}

void 
Stage3::Devices::_PS2Controller::irq12()
{
    uint8_t data = _readData();
    
    if (deviceDrivers[1] != NULL)
        deviceDrivers[1]->recvByte(data);
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic push
void 
Stage3::Devices::_PS2Controller::_irq1(uint64_t intNum)
{
    PS2Controller.irq1();
}

void 
Stage3::Devices::_PS2Controller::_irq12(uint64_t intNum)
{
    PS2Controller.irq12();
}
#pragma GCC diagnostic pop
