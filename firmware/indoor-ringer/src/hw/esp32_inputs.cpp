#include "esp32_inputs.h"

namespace ringer {

Esp32Inputs::Esp32Inputs(int buttonPin, EspNowRings& rings) : button_(buttonPin), rings_(rings) {}

uint32_t Esp32Inputs::nowMs() { return millis(); }

bool Esp32Inputs::buttonDown() {
    if (!pinReady_) {
        pinMode(button_, INPUT_PULLUP);
        pinReady_ = true;
    }
    return digitalRead(button_) == LOW;  // the button pulls GPIO2 to GND
}

bool Esp32Inputs::takeRing(uint32_t& atMs) { return rings_.take(atMs); }

}  // namespace ringer
