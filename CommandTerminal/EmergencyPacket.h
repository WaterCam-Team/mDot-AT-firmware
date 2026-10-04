#ifndef EMERGENCY_PACKET_H
#define EMERGENCY_PACKET_H

#include <stdint.h>
#include <stddef.h>

// The emergency downlink is exactly one byte: '!' (0x21). Nothing else
// counts -- a stray one-byte downlink must not power the Pi on.
// Kept free of mbed headers so tests/test_emergency_packet.cpp can build it
// on a host.
static const uint8_t EMERGENCY_PACKET_BYTE = 0x21;

inline bool is_emergency_payload(const uint8_t *payload, uint16_t size) {
    return payload != NULL && size == 1 && payload[0] == EMERGENCY_PACKET_BYTE;
}

#endif
