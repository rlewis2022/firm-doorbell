// Hardware access: wake reason, the button, waiting, and deep sleep with wake on the button
// (GPIO2, ext0; DL-006). Thin wrapper, no decisions.
#pragma once
#include <Arduino.h>
#include "../logic/ports.h"

namespace doorbell {

class Esp32Power : public PowerCtl {
public:
    explicit Esp32Power(gpio_num_t buttonPin);
    WakeReason wakeReason() override;
    bool buttonHeld() override;
    void waitMs(uint32_t ms) override;
    void sleepUntilButton() override;

private:
    gpio_num_t button_;
    bool pinReady_ = false;
};

}  // namespace doorbell
