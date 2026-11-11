// The ring path (A-05): sound first, then wait for the button to be released, then sleep.
#include "ring.h"
#include "../button.h"

namespace doorbell {

void onWake(const Sound& builtin, AudioOut& audio, PowerCtl& power) {
    const WakeReason reason = power.wakeReason();
    if (reason == WakeReason::Button || reason == WakeReason::PowerOn) {
        audio.play(builtin);  // if playback fails, still sleep (AC5)
        waitForRelease(power, kReleaseTimeoutMs, kReleasePollMs);
    }
    power.sleepUntilButton();
}

}  // namespace doorbell
