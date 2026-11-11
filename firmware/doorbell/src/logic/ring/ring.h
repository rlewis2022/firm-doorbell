// The ring path (absolute A-05): what the Doorbell does each time it wakes.
// Declared in the INC-01 lock commit; behavior is defined by tests/local/locked/test_inc01_ring.cpp.
#pragma once
#include "../ports.h"

namespace doorbell {

// After the sound, wait up to this long for the button to be released, checking this often.
constexpr uint32_t kReleaseTimeoutMs = 10000;
constexpr uint32_t kReleasePollMs = 10;

// Runs once per wake. On a power-on or button wake: play `builtin` first, then wait for the button
// to be released (up to kReleaseTimeoutMs), then sleep until the next press. On any other wake:
// sleep without a sound.
void onWake(const Sound& builtin, AudioOut& audio, PowerCtl& power);

}  // namespace doorbell
