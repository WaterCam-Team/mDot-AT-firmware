#include "CmdClassCPacketProcessor.h"
#include "CommandTerminal.h"
#include "mbed.h"
#include <string>
#include <algorithm>
#include "stm32f4xx_hal.h"  // For HAL GPIO functions
#include "MTSText.h"  // For bin2hexString function

// Static member initialization
bool CmdClassCPacketProcessor::_enabled = false;
DigitalIn CmdClassCPacketProcessor::_emergencyInputPin(PA_6, PullDown); // Set LOW by default with pull-down
DigitalOut CmdClassCPacketProcessor::_emergencyOutputPin(PB_1, 0); // Set LOW by default
Timer CmdClassCPacketProcessor::_emergencyTimer;

// Helper function to reconfigure pin after sleep
static void reconfigureEmergencyPin() {
    // Force pin reconfiguration to ensure proper digital input mode
    // This helps after sleep mode where pins might be in analog mode
    
    // Create a new DigitalIn object to force pin reconfiguration
    // This ensures the pin is in digital input mode with pull-down
    DigitalIn tempPin(PA_6, PullDown);
    
    // Small delay to ensure pin state stabilizes
    ThisThread::sleep_for(50ms);
    
    // Verify pin is working as digital input
    int pinState = tempPin.read();
    CommandTerminal::Serial()->writef("Pin reconfiguration complete. PA_6 state: %d\r\n", pinState);
}

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
            
            // Explicitly configure pin as digital input
            configureEmergencyPin();
            
            // Reconfigure pin after potential sleep mode
            reconfigureEmergencyPin();
            
            // Set device to Class C mode if not already
            if (CommandTerminal::Dot()->getClass() != "C") {
                CommandTerminal::Dot()->setClass("C");
                CommandTerminal::Serial()->writef("Device set to Class C mode\r\n");
            }
            
            // Open continuous receive window
            CommandTerminal::Dot()->openRxWindow(0);
            CommandTerminal::Serial()->writef("Continuous receive window opened\r\n");
            if (_emergencyInputPin.read() == 0) {
                CommandTerminal::Serial()->writef("Input pin is LOW\r\n");
            } else {
                CommandTerminal::Serial()->writef("Input pin is HIGH\r\n");
            }
            if (_emergencyOutputPin.read() == 0) {
                CommandTerminal::Serial()->writef("Output pin is LOW\r\n");
            } else {
                CommandTerminal::Serial()->writef("Output pin is HIGH\r\n");
            }
            
        } else if (enable == 0) {
            _enabled = false;
            CommandTerminal::Dot()->closeRxWindow();
            CommandTerminal::Serial()->writef("Class C packet processor disabled\r\n");
        } else if (enable == 2) {
            // Test mode - manually test the switch
            CommandTerminal::Serial()->writef("Testing PB_1 switch activation...\r\n");
            
            // Force PB_1 configuration
            GPIO_InitTypeDef GPIO_InitStruct;
            __GPIOB_CLK_ENABLE();
            GPIO_InitStruct.Pin = GPIO_PIN_1;
            GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
            HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
            
            // Test switch activation
            CommandTerminal::Serial()->writef("Setting PB_1 HIGH for 2 seconds...\r\n");
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
            _emergencyOutputPin = 1;
            
            int outputState = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);
            CommandTerminal::Serial()->writef("PB_1 state after setting HIGH: %d\r\n", outputState);
            
            ThisThread::sleep_for(2000ms);
            
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
            _emergencyOutputPin = 0;
            
            outputState = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);
            CommandTerminal::Serial()->writef("PB_1 state after setting LOW: %d\r\n", outputState);
            CommandTerminal::Serial()->writef("Switch test complete\r\n");
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
        
        if (enable != 0 && enable != 1 && enable != 2) {
#if MTS_CMD_TERM_VERBOSE
            CommandTerminal::setErrorMessage("Invalid value, expects (0-1) or (2) for test mode");
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
    // Check if input pin PA_6 is HIGH - this means the RPi is ON
    int pinState = _emergencyInputPin.read();
    CommandTerminal::Serial()->writef("EMERGENCY: PA_6 pin state: %d\r\n", pinState);
    
    if (pinState == 0) { // Pin is LOW = RPi is OFF
        CommandTerminal::Serial()->writef("EMERGENCY: Activating switch on PB_1...\r\n");
        
        // Force PB_1 configuration before activation
        GPIO_InitTypeDef GPIO_InitStruct;
        __GPIOB_CLK_ENABLE();
        GPIO_InitStruct.Pin = GPIO_PIN_1;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
        
        // Set output pin PB_1 HIGH for switch activation
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
        _emergencyOutputPin = 1;
        
        // Verify pin is actually HIGH
        int outputState = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);
        CommandTerminal::Serial()->writef("EMERGENCY: PB_1 set HIGH, actual state: %d\r\n", outputState);
        
        // Start timer for activation period
        _emergencyTimer.reset();
        _emergencyTimer.start();
        
        // Log the emergency event
        CommandTerminal::Serial()->writef("EMERGENCY: Input pin LOW, output pin activated\r\n");
        
        // Keep switch active for 1.5 seconds
        while (_emergencyTimer.read_ms() < 1500) {
            ThisThread::sleep_for(10ms);
            // Periodically verify pin is still HIGH
            if (_emergencyTimer.read_ms() % 500 == 0) {
                outputState = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);
                CommandTerminal::Serial()->writef("EMERGENCY: PB_1 state check: %d\r\n", outputState);
            }
        }
        
        // Set output pin back to LOW
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
        _emergencyOutputPin = 0;
        _emergencyTimer.stop();
        
        // Final verification
        outputState = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);
        CommandTerminal::Serial()->writef("EMERGENCY: PB_1 deactivated, final state: %d\r\n", outputState);
        CommandTerminal::Serial()->writef("EMERGENCY: Output pin deactivated\r\n");
    } else {
        // PA_6 is HIGH, RPi is ON, no action needed
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

void CmdClassCPacketProcessor::configureEmergencyPin() {
    // Explicitly configure PA_6 as digital input with pull-down
    // This ensures the pin is in the correct mode regardless of previous state
    
    // Force GPIO configuration for PA_6
    GPIO_InitTypeDef GPIO_InitStruct;
    
    // Enable GPIOA clock
    __GPIOA_CLK_ENABLE();
    
    // Configure PA_6 as digital input with pull-down
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // Small delay for pin to stabilize
    ThisThread::sleep_for(100ms);
    
    // Read and report pin state
    int pinState = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6);
    CommandTerminal::Serial()->writef("PA_6 configured as digital input. Current state: %d\r\n", pinState);
    
    // Also configure PB_1 as digital output
    // Enable GPIOB clock
    __GPIOB_CLK_ENABLE();
    
    // Configure PB_1 as digital output with push-pull
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    // Ensure PB_1 starts LOW
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
    
    CommandTerminal::Serial()->writef("PB_1 configured as digital output. Starting LOW\r\n");
}

void CmdClassCPacketProcessor::startupInit() {
    // Set CPROC=1 (enable Class C packet processor)
    _enabled = true;
    CommandTerminal::Serial()->writef("Class C packet processor enabled on startup\r\n");
    
    // Configure emergency pins
    configureEmergencyPin();
    
    // Set device to Class C mode if not already
    if (CommandTerminal::Dot()->getClass() != "C") {
        CommandTerminal::Dot()->setClass("C");
        CommandTerminal::Serial()->writef("Device set to Class C mode\r\n");
    }
    
    // Open continuous receive window
    CommandTerminal::Dot()->openRxWindow(0);
    CommandTerminal::Serial()->writef("Continuous receive window opened\r\n");
    
    // Wait a moment for the device to stabilize
    ThisThread::sleep_for(1000ms);
}

void CmdClassCPacketProcessor::sendStatusPacketIfNeeded() {
    if (!_enabled) return;
    if (!CommandTerminal::Dot()->getNetworkJoinStatus()) return;
    // Create status packet with device information
    std::string statusMsg = "STATUS:CPROC=1,CLASS=C,READY=1";
    // Add device ID if available
    std::vector<uint8_t> deviceIdVec = CommandTerminal::Dot()->getDeviceId();
    if (!deviceIdVec.empty()) {
        std::string deviceIdHex = mts::Text::bin2hexString(deviceIdVec, "");
        statusMsg += ",DEVICE=" + deviceIdHex;
    }
    // Add network status
    statusMsg += ",NETWORK=JOINED";
    // Convert to vector for sending
    std::vector<uint8_t> statusData(statusMsg.begin(), statusMsg.end());
    // Send the status packet
    if (CommandTerminal::Dot()->send(statusData, false) == mDot::MDOT_OK) {
        CommandTerminal::Serial()->writef("Status packet sent to gateway: %s\r\n", statusMsg.c_str());
    } else {
        CommandTerminal::Serial()->writef("Failed to send status packet: %s\r\n", CommandTerminal::Dot()->getLastError().c_str());
    }
} 