// ESP32 Doorbell firmware (FIRM demo).
// Layers (DL-011): logic in src/logic/ (plain C++), hardware access in src/hw/, this sketch only wires them.
// Each wake starts here from the top: the Doorbell runs onWake() once, then deep-sleeps. loop() never runs.
// Sound first (A-05): the chime starts on this core; once it has, the other core tells the Indoor
// Ringer over ESP-NOW (DL-036). The ring path never waits for the radio.
#include "src/hw/esp32_power.h"
#include "src/hw/espnow_link.h"
#include "src/hw/i2s_audio.h"
#include "src/logic/notify/notify.h"
#include "src/logic/ring/ring.h"
#include "src/sounds/builtin_dingdong.h"

// Wiring: MAX98357A BCLK=GPIO4, LRC=GPIO5, DIN=GPIO6; button GPIO2 to GND. ESP-NOW on channel 1.
doorbell::I2sAudio audio(4, 5, 6);
doorbell::Esp32Power power(GPIO_NUM_2);
doorbell::EspNowLink link(1);

RTC_DATA_ATTR uint16_t ringNumber = 0;  // kept through deep sleep; reset to 0 at power-on
doorbell::WakeReason wakeReason;

void tellRinger() { doorbell::notifyOnWake(wakeReason, ringNumber, link); }
void letRingFinish() { doorbell::waitForAfterSoundJob(200); }

void setup() {
    wakeReason = power.wakeReason();
    doorbell::runAfterSoundStarts(audio, tellRinger, 500);
    power.beforeSleep(letRingFinish);
    doorbell::onWake({builtin_dingdong_wav, builtin_dingdong_wav_len}, audio, power);
}

void loop() {}
