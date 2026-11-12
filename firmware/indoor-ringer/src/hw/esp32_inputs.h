// Hardware access: the clock, the test button (GPIO2 to GND, internal pull-up) and ring messages.
// Thin wrapper, no decisions. Ring messages arrive over ESP-NOW in backlog rank 3; until then
// takeRing() never reports one.
#pragma once
#include <Arduino.h>
#include "../logic/ports.h"

namespace ringer {

class Esp32Inputs : public Inputs {
public:
    explicit Esp32Inputs(int buttonPin);
    uint32_t nowMs() override;
    bool buttonDown() override;
    bool takeRing(uint32_t& atMs) override;

private:
    int button_;
    bool pinReady_ = false;
};

}  // namespace ringer
