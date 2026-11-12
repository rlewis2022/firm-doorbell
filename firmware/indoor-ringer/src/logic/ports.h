// Interfaces the Indoor Ringer's logic uses to reach the hardware (Decide §1.3).
// Declared in the INC-02 lock commit. Logic sees only these; src/hw/ implements them; tests fake them.
// Standard headers only (absolute A-02). Namespace `ringer` keeps it apart from the Doorbell's logic.
#pragma once
#include <stddef.h>
#include <stdint.h>

namespace ringer {

// A complete WAV file held in memory or flash.
struct Sound {
    const uint8_t* data;
    size_t len;
};

class AudioOut {
public:
    virtual ~AudioOut() = default;
    // Plays the whole sound, then returns. Returns false if playback failed.
    virtual bool play(const Sound& sound) = 0;
};

class Inputs {
public:
    virtual ~Inputs() = default;
    // Milliseconds since start-up.
    virtual uint32_t nowMs() = 0;
    // True while the test button (GPIO2) is pressed down. Raw reading, not debounced.
    virtual bool buttonDown() = 0;
    // If a ring message has arrived since the last call, returns true and the time it arrived
    // (nowMs() clock). Rank 3 connects this to ESP-NOW; until then it never reports a ring.
    virtual bool takeRing(uint32_t& atMs) = 0;
};

}  // namespace ringer
