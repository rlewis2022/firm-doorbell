// Hardware access: the clock, the test button (GPIO2 to GND, internal pull-up) and ring messages
// (ESP-NOW, from EspNowRings). Thin wrapper, no decisions.
#pragma once
#include <Arduino.h>
#include "../logic/ports.h"
#include "espnow_rings.h"

namespace ringer {

class Esp32Inputs : public Inputs {
public:
    Esp32Inputs(int buttonPin, EspNowRings& rings);
    uint32_t nowMs() override;
    bool buttonDown() override;
    bool takeRing(uint32_t& atMs) override;

private:
    int button_;
    EspNowRings& rings_;
    bool pinReady_ = false;
};

}  // namespace ringer
