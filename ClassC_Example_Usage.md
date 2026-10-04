# Class C Packet Processor - Usage Guide

## Overview

The Class C Packet Processor provides automatic packet handling for Class C mode mDots with emergency packet detection and serial forwarding capabilities.

## Features

1. **Emergency Packet Detection**: Automatically detects packets containing "emergency" (case-insensitive)
2. **Input Pin Monitoring**: Checks PA_6 pin status when emergency packets are received
3. **Output Pin Control**: Sets PB_1 HIGH for 500ms when emergency input pin is LOW
4. **Serial Forwarding**: Forwards non-emergency packets to host computer via serial
5. **Automatic Class C Mode**: Automatically configures device for Class C operation

## Hardware Requirements

- **Input Pin**: PA_6 (emergency status input, defaults to LOW with pull-down)
- **Output Pin**: PB_1 (emergency response output, defaults to LOW)
- **Serial Interface**: Standard UART for packet forwarding

## Pin Configuration

### PA_6 (Emergency Input Pin)
- **Default State**: LOW (with internal pull-down resistor)
- **Purpose**: Indicates if emergency is already acknowledged
- **Logic**: 
  - LOW = Emergency not acknowledged (trigger response)
  - HIGH = Emergency already acknowledged (no response)
- **Configuration**: Internal pull-down resistor ensures LOW by default
- **Note**: PA_6 is chosen for reliability as it's not part of the UART or XBEE interface

### PB_1 (Emergency Output Pin)
- **Default State**: LOW
- **Purpose**: Emergency response output signal
- **Logic**:
  - LOW = Normal state (no emergency response)
  - HIGH = Emergency response active (500ms duration)
- **Configuration**: Explicitly initialized to LOW by default

## AT Commands

### Enable Class C Packet Processor
```
AT+CPROC=1
```
This command:
- Enables the packet processor
- Sets device to Class C mode
- Opens continuous receive window
- Returns confirmation messages

### Disable Class C Packet Processor
```
AT+CPROC=0
```
This command:
- Disables the packet processor
- Closes receive window
- Returns confirmation message

### Query Status
```
AT+CPROC?
```
Returns current status (0 = disabled, 1 = enabled)

### Test Switch Activation
```
AT+CPROC=2
```
This command:
- Tests PB_1 switch activation manually
- Sets PB_1 HIGH for 2 seconds
- Reports pin states for debugging
- Useful for troubleshooting switch issues

## Emergency Packet Handling

### Emergency Packet Format
Any packet containing the word "emergency" (case-insensitive) will trigger emergency handling:

**Examples:**
- `"emergency"` - Direct emergency message
- `"This is an emergency situation"` - Emergency in context
- `"EMERGENCY ALERT"` - Uppercase emergency
- `"Emergency broadcast"` - Mixed case emergency

### Emergency Response Logic

1. **Packet Received**: System detects "emergency" in packet content
2. **Input Check**: Reads PA_6 pin status
3. **Response Action**:
   - If PA_6 is **LOW**: Sets PB_1 HIGH for 500ms, then returns to LOW
   - If PA_6 is **HIGH**: No action taken (emergency already acknowledged)
4. **Logging**: All emergency events are logged to serial

### Emergency Response Timing
- **Output Duration**: 500ms (0.5 seconds)
- **Response Time**: Immediate upon packet reception
- **Logging**: Real-time status messages

## Serial Packet Forwarding

### Non-Emergency Packet Format
Packets that don't contain "emergency" are automatically forwarded to serial in this format:

```
PACKET:PORT=<port>,SIZE=<size>,DATA=<hex_data>
```

**Example:**
```
PACKET:PORT=1,SIZE=8,DATA=48656C6C6F202121
```

### Packet Information
- **PORT**: Application port number (0-255)
- **SIZE**: Packet payload size in bytes
- **DATA**: Packet payload as hexadecimal string

## Usage Examples

### Basic Setup
```
AT+CPROC=1
```
Response:
```
Class C packet processor enabled
Device set to Class C mode
Continuous receive window opened
OK
```

### Emergency Scenario
1. **Emergency Packet Received**: `"emergency alert"`
2. **PA_6 Status Check**: If LOW (emergency not acknowledged)
3. **PB_1 Activation**: Set HIGH for 500ms
4. **Serial Output**:
   ```
   EMERGENCY: Input pin LOW, output pin activated
   EMERGENCY: Output pin deactivated
   ```

### Normal Packet Forwarding
1. **Regular Packet Received**: `"Hello World"`
2. **Serial Output**:
   ```
   PACKET:PORT=1,SIZE=11,DATA=48656C6C6F20576F726C64
   ```

### Emergency Already Acknowledged
1. **Emergency Packet Received**: `"emergency"`
2. **PA_6 Status**: HIGH (already acknowledged)
3. **Serial Output**:
   ```
   EMERGENCY: Input pin HIGH, no action taken
   ```

## Integration with Existing Commands

The Class C Packet Processor works alongside existing AT commands:

- **AT+RECV**: Still works for manual packet reception
- **AT+RECVC**: Still works for continuous reception
- **AT+URC**: Can be used together for additional packet notifications
- **AT+SD**: Serial data mode still functional

## Error Handling

- **Invalid Arguments**: Returns ERROR for invalid enable/disable values
- **Network Issues**: Continues operation even if network join fails
- **Serial Buffer Full**: Waits for buffer space before forwarding packets
- **Pin Access Errors**: Logs errors but continues packet processing

## Power Considerations

- **Class C Mode**: Device stays in receive mode, higher power consumption
- **Continuous Reception**: Radio remains active for packet reception
- **Pin Monitoring**: Minimal power impact from GPIO operations
- **Serial Forwarding**: Power usage depends on packet frequency

## Troubleshooting

### Common Issues

1. **No Packets Received**:
   - Check network join status: `AT+NJS?`
   - Verify Class C mode: `AT+DC?`
   - Check receive window: `AT+CPROC?`

2. **Emergency Not Triggering**:
   - Verify packet contains "emergency" (case-insensitive)
   - Check PA_6 pin connection and status
   - Monitor serial for emergency log messages

3. **Serial Forwarding Issues**:
   - Check serial connection and baud rate
   - Verify packet processor is enabled
   - Monitor for buffer overflow messages

### Debug Commands
```
AT+NJS?     # Check network join status
AT+DC?      # Check device class
AT+CPROC?   # Check processor status
AT+URC?     # Check unsolicited response codes
AT+CPROC=2  # Test switch activation manually
```

## Switch Activation Troubleshooting

### Common Switch Issues

1. **Switch Not Activating**:
   - **Test manually**: Use `AT+CPROC=2` to test switch activation
   - **Check voltage**: Ensure switch operates at 3.3V logic level
   - **Check current**: Some switches need more than 25mA
   - **Verify connections**: Check wiring and connections

2. **Insufficient Current**:
   - **Add transistor**: Use a transistor to boost current drive
   - **Use relay module**: Consider a relay module with built-in driver
   - **Check switch specs**: Verify switch activation requirements

3. **Timing Issues**:
   - **Increase duration**: Switch may need longer activation time
   - **Add delay**: Some switches need time to activate
   - **Check switch type**: Different switches have different timing requirements

4. **Pin Configuration Issues**:
   - **Sleep mode**: Pin may be in analog mode after sleep
   - **Pin conflicts**: Ensure PB_1 is not used by other functions
   - **GPIO state**: Pin may not restore properly after sleep

### Switch Testing Procedure

1. **Manual Test**:
   ```
   AT+CPROC=2
   ```
   This will activate the switch for 2 seconds and report pin states.

2. **Monitor Output**:
   - Check serial output for pin state reports
   - Verify PB_1 goes HIGH (1) and then LOW (0)
   - Look for any error messages

3. **Hardware Verification**:
   - Use multimeter to measure PB_1 voltage
   - Should read ~3.3V when HIGH, ~0V when LOW
   - Check switch connections and power supply

### Recommended Switch Types

- **Solid State Relays**: Good for low-power applications
- **Mechanical Relays**: For higher current requirements
- **Optocoupler Switches**: For isolation requirements
- **Transistor Arrays**: For multiple switch control

## Advanced Configuration

### Custom Emergency Detection
To modify emergency detection logic, edit the `isEmergencyPacket()` function in `CmdClassCPacketProcessor.cpp`.

### Custom Response Timing
To change the 500ms response duration, modify the timer value in `handleEmergencyPacket()`.

### Custom Serial Format
To change packet forwarding format, modify the `forwardPacketToSerial()` function.

## Security Considerations

- **Packet Content**: All packet content is processed locally
- **Pin Access**: Direct hardware access for emergency response
- **Serial Data**: Packet data forwarded in plain text
- **Network Security**: Relies on LoRaWAN network security

## Performance Notes

- **Packet Processing**: Minimal latency for emergency detection
- **Serial Throughput**: Limited by serial baud rate
- **Memory Usage**: Static allocation for packet buffers
- **CPU Usage**: Low overhead for packet processing 