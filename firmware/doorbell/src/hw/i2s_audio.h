// Hardware access: plays a WAV through the MAX98357A over I2S, with the core's own ESP_I2S
// playWAV() (DL-007, A-08). Thin wrapper, no decisions.
// The amp is switched by its SD_MODE pin (DL-039): high only while a sound plays, low otherwise
// (low = shut down, 0.6 uA typical instead of 2.4 mA idle, MAX98357A data sheet).
#pragma once
#include <ESP_I2S.h>
#include "../logic/ports.h"

namespace doorbell {

class I2sAudio : public AudioOut {
public:
    I2sAudio(int bclkPin, int lrcPin, int dinPin, int ampEnablePin);
    bool play(const Sound& sound) override;
    // Called once per play(), just before the sound starts (or when playback can't start), so radio
    // work can wait for the sound (sound first, A-05; DL-036).
    void onStart(void (*fn)()) { onStart_ = fn; }
    // Holds the amp shut down (SD_MODE low) through deep sleep. Call just before sleeping.
    void holdAmpOffForSleep();

private:
    I2SClass i2s_;
    void ampOn();
    void ampOff();
    int bclk_, lrc_, din_, amp_;
    bool started_ = false;
    void (*onStart_)() = nullptr;
};

}  // namespace doorbell
