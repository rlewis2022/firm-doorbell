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

// Wiring: MAX98357A BCLK=GPIO4, LRC=GPIO5, DIN=GPIO6, SD_MODE=GPIO7 (amp on/off), VIN on the battery (B+);
// button GPIO2 to GND. ESP-NOW on channel 1.
doorbell::I2sAudio audio(4, 5, 6, 7);
doorbell::Esp32Power power(GPIO_NUM_2);
doorbell::EspNowLink radio(1);  // not "link": POSIX link() exists

RTC_DATA_ATTR uint16_t ringNumber = 0;  // kept through deep sleep; reset to 0 at power-on
doorbell::WakeReason wakeReason;

void tellRinger() { doorbell::notifyOnWake(wakeReason, ringNumber, radio); }
void beforeSleep() {
    doorbell::waitForAfterSoundJob(200);  // let the ring message finish sending
    audio.holdAmpOffForSleep();           // amp off through deep sleep (DL-039)
}

void setup() {
    wakeReason = power.wakeReason();
    doorbell::runAfterSoundStarts(audio, tellRinger, 500);
    power.beforeSleep(beforeSleep);
    doorbell::onWake({builtin_dingdong_wav, builtin_dingdong_wav_len}, audio, power);
}

void loop() {}
