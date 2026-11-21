#include "i2s_audio.h"
#include "driver/gpio.h"

namespace doorbell {

namespace {
// MAX98357A turn-on time after SD_MODE goes high: 7 ms typical, 7.5 ms max (data sheet). Without the
// wait the start of the chime would be cut off.
constexpr uint32_t kAmpTurnOnMs = 8;
}

I2sAudio::I2sAudio(int bclkPin, int lrcPin, int dinPin, int ampEnablePin)
    : bclk_(bclkPin), lrc_(lrcPin), din_(dinPin), amp_(ampEnablePin) {}

void I2sAudio::ampOn() {
    gpio_hold_dis(static_cast<gpio_num_t>(amp_));  // released from the deep-sleep hold
    pinMode(amp_, OUTPUT);
    digitalWrite(amp_, HIGH);  // above 1.4 V: amp on (left channel; the sound is mono)
}

void I2sAudio::ampOff() {
    pinMode(amp_, OUTPUT);
    digitalWrite(amp_, LOW);  // amp shut down
}

void I2sAudio::holdAmpOffForSleep() {
    ampOff();
    gpio_hold_en(static_cast<gpio_num_t>(amp_));  // GPIO7 is an RTC pin: the hold lasts through deep sleep
#if SOC_GPIO_SUPPORT_HOLD_IO_IN_DSLP && !SOC_GPIO_SUPPORT_HOLD_SINGLE_IO_IN_DSLP
    gpio_deep_sleep_hold_en();
#endif
}

bool I2sAudio::play(const Sound& sound) {
    const uint32_t ampOnAt = millis();
    ampOn();
    if (!started_) {
        i2s_.setPins(bclk_, lrc_, din_);
        // Matches budget B-02 (24 kHz, 16-bit, mono). playWAV() reads the WAV's own header too.
        started_ = i2s_.begin(I2S_MODE_STD, 24000, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO);
    }
    const bool ok = started_ && sound.data != nullptr && sound.len >= 44;
    if (!ok) {
        ampOff();
        if (onStart_) onStart_();  // no sound can start: radio work may go now
        return false;
    }
    while (millis() - ampOnAt < kAmpTurnOnMs) delay(1);  // let the amp finish turning on
    if (onStart_) onStart_();  // the sound starts now: radio work may go (sound first, A-05)
    // playWAV() only reads the buffer, but its parameter isn't const.
    i2s_.playWAV(const_cast<uint8_t*>(sound.data), sound.len);  // blocks until the sound ends
    ampOff();
    return true;
}

}  // namespace doorbell
