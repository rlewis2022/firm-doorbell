// Hardware access: sends ring messages to the Indoor Ringer by ESP-NOW broadcast on channel 1
// (DL-036, DL-037). Station mode only, never joins a network (A-03). Thin wrapper, no decisions.
#pragma once
#include "../logic/ports.h"

namespace doorbell {

class EspNowLink : public RingLink {
public:
    explicit EspNowLink(uint8_t channel) : channel_(channel) {}
    // Starts the radio and ESP-NOW on first use, then queues the message for broadcast.
    bool send(const uint8_t* msg, size_t len) override;

private:
    bool start();
    uint8_t channel_;
    bool started_ = false;
};

// Runs `job` once on processor core 0 after the chime has started (I2sAudio::onStart), so radio work
// never delays the sound (A-05, DL-036). If no sound starts within `timeoutMs` (a wake with no
// chime), `job` runs anyway.
class I2sAudio;
void runAfterSoundStarts(I2sAudio& audio, void (*job)(), uint32_t timeoutMs);
// Waits up to `maxMs` for that job to finish. Returns at once if no job was started.
void waitForAfterSoundJob(uint32_t maxMs);

}  // namespace doorbell
