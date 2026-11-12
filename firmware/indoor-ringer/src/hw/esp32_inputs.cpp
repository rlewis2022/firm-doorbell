#include "esp32_inputs.h"

namespace ringer {

Esp32Inputs::Esp32Inputs(int buttonPin) : button_(buttonPin) {}

uint32_t Esp32Inputs::nowMs() { return millis(); }

bool Esp32Inputs::buttonDown() {
    if (!pinReady_) {
        pinMode(button_, INPUT_PULLUP);
        pinReady_ = true;
    }
    return digitalRead(button_) == LOW;  // the button pulls GPIO2 to GND
}

bool Esp32Inputs::takeRing(uint32_t& atMs) {
    (void)atMs;
    return false;  // no radio yet: ESP-NOW ring messages come with backlog rank 3
}

}  // namespace ringer
