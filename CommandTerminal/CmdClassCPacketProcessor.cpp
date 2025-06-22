#include "CmdClassCPacketProcessor.h"
#include "CommandTerminal.h"
#include "mbed.h"
#include <string>
#include <algorithm>

// Static member initialization
bool CmdClassCPacketProcessor::_enabled = false;
DigitalIn CmdClassCPacketProcessor::_emergencyInputPin(PC_1);
DigitalOut CmdClassCPacketProcessor::_emergencyOutputPin(PB_1);
Timer CmdClassCPacketProcessor::_emergencyTimer;

CmdClassCPacketProcessor::CmdClassCPacketProcessor() :
#if MTS_CMD_TERM_VERBOSE
    Command("Class C Packet Processor", "AT+CPROC", "Enable/disable Class C packet processing with emergency handling", "(0-1)")
#else
    Command("AT+CPROC")
#endif
{
    _queryable = true;
}

uint32_t CmdClassCPacketProcessor::action(const std::vector<std::string>& args) {
    if (args.size() == 1) {
        // Query current status
        CommandTerminal::Serial()->writef("%d\r\n", _enabled ? 1 : 0);
    } else if (args.size() == 2) {
        int enable;
        sscanf(args[1].c_str(), "%d", &enable);
        
        if (enable == 1) {
            _enabled = true;
            CommandTerminal::Serial()->writef("Class C packet processor enabled\r\n");
            
            // Set device to Class C mode if not already
            if (CommandTerminal::Dot()->getClass() != "C") {
                CommandTerminal::Dot()->setClass("C");
                CommandTerminal::Serial()->writef("Device set to Class C mode\r\n");
            }
            
            // Open continuous receive window
            CommandTerminal::Dot()->openRxWindow(0);
            CommandTerminal::Serial()->writef("Continuous receive window opened\r\n");
            
        } else if (enable == 0) {
            _enabled = false;
            CommandTerminal::Dot()->closeRxWindow();
            CommandTerminal::Serial()->writef("Class C packet processor disabled\r\n");
        }
    }
    
    return 0;
}

bool CmdClassCPacketProcessor::verify(const std::vector<std::string>& args) {
    if (args.size() == 1) {
        return true;
    }
    
    if (args.size() == 2) {
        int enable;
        if (sscanf(args[1].c_str(), "%d", &enable) != 1) {
#if MTS_CMD_TERM_VERBOSE
            CommandTerminal::setErrorMessage("Invalid argument, expects (0-1)");
#endif
            return false;
        }
        
        if (enable != 0 && enable != 1) {
#if MTS_CMD_TERM_VERBOSE
            CommandTerminal::setErrorMessage("Invalid value, expects (0-1)");
#endif
            return false;
        }
        
        return true;
    }
    
#if MTS_CMD_TERM_VERBOSE
    CommandTerminal::setErrorMessage("Invalid arguments");
#endif
    return false;
}

void CmdClassCPacketProcessor::processIncomingPacket(uint8_t port, uint8_t *payload, uint16_t size) {
    if (!_enabled) {
        return;
    }
    
    // Check if this is an emergency packet
    if (isEmergencyPacket(payload, size)) {
        handleEmergencyPacket();
    } else {
        // Forward non-emergency packets to serial
        forwardPacketToSerial(port, payload, size);
    }
}

bool CmdClassCPacketProcessor::isEmergencyPacket(uint8_t *payload, uint16_t size) {
    if (size < 9) { // "emergency" is 9 characters
        return false;
    }
    
    // Convert payload to string for comparison
    std::string packetStr(reinterpret_cast<char*>(payload), size);
    
    // Check if packet contains "emergency" (case-insensitive)
    std::transform(packetStr.begin(), packetStr.end(), packetStr.begin(), ::tolower);
    
    return packetStr.find("emergency") != std::string::npos;
}

void CmdClassCPacketProcessor::handleEmergencyPacket() {
    // Check if input pin PC_1 is HIGH - this means the RPi is ON
    if (_emergencyInputPin.read() == 0) { // Pin is LOW = RPi is OFF
        // Set output pin PB_1 HIGH for half a second
        _emergencyOutputPin = 1;
        
        // Start timer for half second
        _emergencyTimer.reset();
        _emergencyTimer.start();
        
        // Log the emergency event
        CommandTerminal::Serial()->writef("EMERGENCY: Input pin LOW, output pin activated\r\n");
        
        // Wait for half second (500ms)
        while (_emergencyTimer.read_ms() < 500) {
            ThisThread::sleep_for(10ms);
        }
        
        // Set output pin back to LOW
        _emergencyOutputPin = 0;
        _emergencyTimer.stop();
        
        CommandTerminal::Serial()->writef("EMERGENCY: Output pin deactivated\r\n");
    } else {
        // PC_1 is HIGH, RPi is ON, no action needed
        CommandTerminal::Serial()->writef("EMERGENCY: Input pin HIGH, no action taken\r\n");
    }
}

void CmdClassCPacketProcessor::forwardPacketToSerial(uint8_t port, uint8_t *payload, uint16_t size) {
    // Format packet for serial transmission
    std::string packetData = "PACKET:";
    packetData += "PORT=" + std::to_string(port) + ",";
    packetData += "SIZE=" + std::to_string(size) + ",";
    packetData += "DATA=";
    
    // Add payload data as hex string
    for (uint16_t i = 0; i < size; i++) {
        char hex[3];
        snprintf(hex, sizeof(hex), "%02X", payload[i]);
        packetData += hex;
    }
    
    packetData += "\r\n";
    
    // Send to serial using write() with proper length
    CommandTerminal::Serial()->write(packetData.c_str(), packetData.length());
} 