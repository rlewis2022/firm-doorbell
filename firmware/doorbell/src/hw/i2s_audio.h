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

private:
    I2SClass i2s_;
    int bclk_, lrc_, din_;
    bool started_ = false;
};

}  // namespace doorbell
