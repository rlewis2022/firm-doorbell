// Hardware access: plays a WAV through the MAX98357A over I2S, with the core's own ESP_I2S
// playWAV() (DL-007, A-08). Thin wrapper, no decisions.
#pragma once
#include <ESP_I2S.h>
#include "../logic/ports.h"

namespace doorbell {

class I2sAudio : public AudioOut {
public:
    I2sAudio(int bclkPin, int lrcPin, int dinPin);
    bool play(const Sound& sound) override;
    // Called once per play(), just before the sound starts (or when playback can't start), so radio
    // work can wait for the sound (sound first, A-05; DL-036).
    void onStart(void (*fn)()) { onStart_ = fn; }

private:
    I2SClass i2s_;
    int bclk_, lrc_, din_;
    bool started_ = false;
    void (*onStart_)() = nullptr;
};

}  // namespace doorbell
