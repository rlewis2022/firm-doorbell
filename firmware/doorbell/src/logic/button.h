// Waiting for the doorbell button to be released (INC-01, AC6/AC7). Plain C++ (A-02).
#pragma once
#include "ports.h"

namespace doorbell {

// Waits until the button is released, or until timeoutMs has passed, checking every pollMs.
// Returns true if it was released, false if it gave up.
bool waitForRelease(PowerCtl& power, uint32_t timeoutMs, uint32_t pollMs);

}  // namespace doorbell
