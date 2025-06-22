#ifndef __CMDCLASSCPACKETPROCESSOR_H__
#define __CMDCLASSCPACKETPROCESSOR_H__

#include "Command.h"

class CommandTerminal;

class CmdClassCPacketProcessor : public Command {

public:

    CmdClassCPacketProcessor();
    virtual uint32_t action(const std::vector<std::string>& args);
    virtual bool verify(const std::vector<std::string>& args);
    
    // Static method to process incoming packets
    static void processIncomingPacket(uint8_t port, uint8_t *payload, uint16_t size);
    
private:
    static bool isEmergencyPacket(uint8_t *payload, uint16_t size);
    static void handleEmergencyPacket();
    static void forwardPacketToSerial(uint8_t port, uint8_t *payload, uint16_t size);
    
    static bool _enabled;
    static DigitalIn _emergencyInputPin;
    static DigitalOut _emergencyOutputPin;
    static Timer _emergencyTimer;
};

#endif // __CMDCLASSCPACKETPROCESSOR_H__ 