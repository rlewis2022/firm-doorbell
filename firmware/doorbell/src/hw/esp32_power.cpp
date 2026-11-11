#include "esp32_power.h"
#include "driver/rtc_io.h"
#include "esp_sleep.h"

namespace doorbell {

Esp32Power::Esp32Power(gpio_num_t buttonPin) : button_(buttonPin) {}

WakeReason Esp32Power::wakeReason() {
    switch (esp_sleep_get_wakeup_cause()) {
        case ESP_SLEEP_WAKEUP_EXT0:      return WakeReason::Button;
        case ESP_SLEEP_WAKEUP_UNDEFINED: return WakeReason::PowerOn;  // power-on or reset
        default:                         return WakeReason::Other;
    }
}

bool Esp32Power::buttonHeld() {
    if (!pinReady_) {
        // After an ext0 wake the pin is still under RTC control; hand it back to normal GPIO.
        rtc_gpio_deinit(button_);
        pinMode(button_, INPUT_PULLUP);
        pinReady_ = true;
    }
    return digitalRead(button_) == LOW;  // button pulls GPIO2 to GND
}

void Esp32Power::waitMs(uint32_t ms) { delay(ms); }

void Esp32Power::sleepUntilButton() {
    rtc_gpio_pullup_en(button_);
    rtc_gpio_pulldown_dis(button_);
    esp_sleep_enable_ext0_wakeup(button_, 0);  // wake when the button pulls GPIO2 LOW
    esp_deep_sleep_start();                    // never returns
}

}  // namespace doorbell
