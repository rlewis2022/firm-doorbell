// The Indoor Ringer's ring path (absolute A-05). Declared in the INC-02 lock commit;
// behavior is defined by tests/local/locked/test_inc02_ringer.cpp.
#pragma once
#include "../ports.h"

namespace ringer {

// The button must read the same for this long before a change counts (filters contact bounce).
constexpr uint32_t kDebounceMs = 30;

class Ringer {
public:
    Ringer(const Sound& builtin, AudioOut& audio, Inputs& inputs);
    // Call once at power-on: plays the built-in sound (DL-027).
    void start();
    // Call over and over. Plays the built-in sound once for each debounced press of the test button,
    // and once for each ring message, except a ring that arrived while a sound was playing (ignored).
    void step();

private:
    void ring();
    const Sound builtin_;
    AudioOut& audio_;
    Inputs& in_;
    bool stableDown_ = false;   // debounced button state
    bool lastRaw_ = false;
    uint32_t rawSinceMs_ = 0;   // when the raw reading last changed
    uint32_t quietFromMs_ = 0;  // when the last sound finished; rings that arrived before this are ignored
};

}  // namespace ringer
