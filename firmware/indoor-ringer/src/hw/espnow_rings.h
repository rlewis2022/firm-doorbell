// Hardware access: listens for ring messages from the Doorbell over ESP-NOW on one channel
// (DL-036, DL-037). Each message is decoded with the shared contract (A-07) and logic decides whether
// it rings (ringsFor, DL-038); rings are queued with their arrival time for Inputs::takeRing().
// Thin wrapper, no decisions. Reports what it hears on the serial port (RT-04).
#pragma once
#include <Arduino.h>

namespace ringer {

class EspNowRings {
public:
    // Starts the radio in station mode (no network joined) and ESP-NOW on `channel`.
    bool begin(uint8_t channel);
    // If a ring has arrived, returns true and its arrival time (millis()). Prints what was heard.
    bool take(uint32_t& atMs);
};

}  // namespace ringer
