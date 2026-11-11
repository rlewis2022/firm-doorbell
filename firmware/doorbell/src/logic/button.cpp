#include "button.h"

namespace doorbell {

bool waitForRelease(PowerCtl& power, uint32_t timeoutMs, uint32_t pollMs) {
    for (uint32_t waited = 0; power.buttonHeld(); waited += pollMs) {
        if (waited >= timeoutMs) return false;  // stuck button: give up (AC7)
        power.waitMs(pollMs);
    }
    return true;
}

}  // namespace doorbell
