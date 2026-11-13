// The Indoor Ringer's ring path (A-05): the built-in sound at power-on, on each debounced test-button
// press, and on each ring message, except a ring that arrived while a sound was playing.
#include "ringer.h"

namespace ringer {

Ringer::Ringer(const Sound& builtin, AudioOut& audio, Inputs& inputs)
    : builtin_(builtin), audio_(audio), in_(inputs) {}

void Ringer::start() {
    lastRaw_ = in_.buttonDown();
    stableDown_ = lastRaw_;  // a button already held at power-on isn't a press
    rawSinceMs_ = in_.nowMs();
    ring();
}

void Ringer::step() {
    const uint32_t now = in_.nowMs();
    const bool raw = in_.buttonDown();
    if (raw != lastRaw_) {
        lastRaw_ = raw;
        rawSinceMs_ = now;
    } else if (raw != stableDown_ && now - rawSinceMs_ >= kDebounceMs) {
        stableDown_ = raw;
        if (stableDown_) ring();  // a debounced press
    }

    uint32_t at = 0;
    while (in_.takeRing(at)) {
        // Signed difference, so this still works when the millisecond clock wraps (~49 days).
        if (static_cast<int32_t>(at - quietFromMs_) >= 0) ring();
    }
}

void Ringer::ring() {
    audio_.play(builtin_);  // blocks until the sound ends; if it fails, carry on (AC9)
    quietFromMs_ = in_.nowMs();
}

}  // namespace ringer
