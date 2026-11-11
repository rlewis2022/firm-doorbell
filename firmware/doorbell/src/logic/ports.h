// Interfaces the Doorbell's logic uses to reach the hardware (Decide §1.2).
// Declared in the INC-01 lock commit. Logic sees only these; src/hw/ implements them; tests fake them.
// Standard headers only (absolute A-02).
#pragma once
#include <stddef.h>
#include <stdint.h>

namespace doorbell {

// Why the chip is awake.
enum class WakeReason {
    PowerOn,  // power connected or reset pressed
    Button,   // the doorbell button (GPIO2, ext0 wake)
    Other     // anything else
};

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

class PowerCtl {
public:
    virtual ~PowerCtl() = default;
    virtual WakeReason wakeReason() = 0;
    // True while the button is pressed down.
    virtual bool buttonHeld() = 0;
    // Waits this many milliseconds.
    virtual void waitMs(uint32_t ms) = 0;
    // Deep sleep until the button is pressed. On the real board this never returns.
    virtual void sleepUntilButton() = 0;
};

}  // namespace doorbell
