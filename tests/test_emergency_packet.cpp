// Host-side test for the emergency-packet check (no mbed needed):
//   g++ -std=c++11 -I CommandTerminal tests/test_emergency_packet.cpp -o /tmp/t && /tmp/t
// tests/.mbedignore keeps this file out of the firmware build.
#include <cstdio>
#include <cstring>
#include "EmergencyPacket.h"

static int failures = 0;

static void check(bool got, bool want, const char *what) {
    if (got != want) {
        std::printf("FAIL: %s (got %d, want %d)\n", what, got, want);
        failures++;
    }
}

int main() {
    const uint8_t bang[] = {'!'};
    check(is_emergency_payload(bang, 1), true, "'!' is an emergency");

    for (int b = 0; b < 256; b++) {          // every other single byte is not
        if (b == '!') continue;
        uint8_t one[] = {(uint8_t)b};
        char what[48];
        std::snprintf(what, sizeof what, "single byte 0x%02x is not an emergency", b);
        check(is_emergency_payload(one, 1), false, what);
    }

    const uint8_t two[] = {'!', '!'};
    check(is_emergency_payload(two, 2), false, "'!!' is not an emergency");
    const uint8_t debug_cmd[] = {0x50, 0x01};
    check(is_emergency_payload(debug_cmd, 2), false, "the 0x50 0x01 debug command is not");
    check(is_emergency_payload(bang, 0), false, "an empty payload is not");
    check(is_emergency_payload(NULL, 1), false, "a null payload is not");

    if (failures == 0) std::printf("ok: emergency check accepts only '!'\n");
    return failures ? 1 : 0;
}
