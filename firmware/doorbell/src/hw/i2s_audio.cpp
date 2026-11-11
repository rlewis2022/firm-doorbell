#include "i2s_audio.h"

namespace doorbell {

I2sAudio::I2sAudio(int bclkPin, int lrcPin, int dinPin) : bclk_(bclkPin), lrc_(lrcPin), din_(dinPin) {}

bool I2sAudio::play(const Sound& sound) {
    if (!started_) {
        i2s_.setPins(bclk_, lrc_, din_);
        // Matches budget B-02 (24 kHz, 16-bit, mono). playWAV() reads the WAV's own header too.
        started_ = i2s_.begin(I2S_MODE_STD, 24000, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO);
    }
    if (!started_ || sound.data == nullptr || sound.len < 44) return false;
    // playWAV() only reads the buffer, but its parameter isn't const.
    i2s_.playWAV(const_cast<uint8_t*>(sound.data), sound.len);  // blocks until the sound ends
    return true;
}

}  // namespace doorbell
