// ESP32 Doorbell firmware (FIRM demo).
// Layers (DL-011): logic in src/logic/ (plain C++), hardware access in src/hw/, this sketch only wires them.
// Each wake starts here from the top: the Doorbell runs onWake() once, then deep-sleeps. loop() never runs.
#include "src/hw/esp32_power.h"
#include "src/hw/i2s_audio.h"
#include "src/logic/ring/ring.h"
#include "src/sounds/builtin_dingdong.h"

// Wiring: MAX98357A BCLK=GPIO4, LRC=GPIO5, DIN=GPIO6; button GPIO2 to GND.
doorbell::I2sAudio audio(4, 5, 6);
doorbell::Esp32Power power(GPIO_NUM_2);

void setup() {
    doorbell::onWake({builtin_dingdong_wav, builtin_dingdong_wav_len}, audio, power);
}

void loop() {}
