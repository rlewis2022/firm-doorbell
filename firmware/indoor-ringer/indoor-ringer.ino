// Indoor Ringer firmware (FIRM demo).
// Layers (DL-011): logic in src/logic/ (plain C++), hardware access in src/hw/, this sketch only wires them.
// USB powered: it stays awake. start() chimes once at power-on; step() runs over and over.
#include "src/hw/esp32_inputs.h"
#include "src/hw/i2s_audio.h"
#include "src/logic/ring/ringer.h"
#include "src/sounds/builtin_dingdong.h"

// Wiring: MAX98357A BCLK=GPIO4, LRC=GPIO5, DIN=GPIO6, GAIN 100 kOhm to GND; test button GPIO2 to GND.
ringer::I2sAudio audio(4, 5, 6);
ringer::Esp32Inputs inputs(2);
ringer::Ringer bell({builtin_dingdong_wav, builtin_dingdong_wav_len}, audio, inputs);

void setup() {
    bell.start();
}

void loop() {
    bell.step();
}
