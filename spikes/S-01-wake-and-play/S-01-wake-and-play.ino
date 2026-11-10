// SPIKE S-01: wake and play — THROWAWAY CODE. Never merged into the product.
// Question: can a button press wake the ESP32-S3 SuperMini from deep sleep and play a
// built-in sound through the I2S amp (NS4168 board from the NULLLAB kit; MAX98357A also works)
// using the core's ESP_I2S playWAV()? How fast?
// Target: Arduino ESP32 core 3.3.12, board "ESP32S3 Dev Module", USB CDC On Boot = Enabled.

#include <ESP_I2S.h>
#include "driver/rtc_io.h"
#include "esp_sleep.h"
#include "esp_timer.h"

// ---- Pins (all on the side headers; GPIO2 is an RTC pin, so it can wake from deep sleep) ----
const gpio_num_t BUTTON_PIN = GPIO_NUM_2;   // button between GPIO2 and GND
const int I2S_BCLK = 4;                     // amp BCL / BCLK
const int I2S_LRC  = 5;                     // amp LRC (word select)
const int I2S_DIN  = 6;                     // amp DIN

// ---- Built-in sound: a two-tone "ding-dong", 16 kHz, 16-bit, STEREO ----
// Stereo on purpose: the NS4168 plays only ONE channel (its CTRL pin selects which), so we put
// the same sample in both channels and it plays whichever one the board is wired for.
const uint32_t SAMPLE_RATE = 16000;
const float TONE1_HZ = 659.25f, TONE2_HZ = 523.25f;   // E5 then C5
const float TONE_SECONDS = 0.35f;
const int16_t AMPLITUDE = 6000;                       // keep it modest on a breadboard

RTC_DATA_ATTR uint32_t wakeCount = 0;  // survives deep sleep

I2SClass i2s;

// Build a PCM WAV (header + samples) in memory, so we exercise playWAV() exactly as the
// product would with a stored file.
static uint8_t *buildDingDongWav(size_t *outLen) {
  const uint32_t samplesPerTone = (uint32_t)(SAMPLE_RATE * TONE_SECONDS);
  const uint32_t numSamples = samplesPerTone * 2;
  const uint32_t dataBytes = numSamples * 4;          // 2 channels x 2 bytes
  const size_t total = 44 + dataBytes;
  uint8_t *buf = (uint8_t *)malloc(total);
  if (!buf) return nullptr;

  auto put16 = [&](size_t at, uint16_t v) { buf[at] = v & 0xFF; buf[at + 1] = v >> 8; };
  auto put32 = [&](size_t at, uint32_t v) { for (int i = 0; i < 4; i++) buf[at + i] = (v >> (8 * i)) & 0xFF; };
  memcpy(buf + 0, "RIFF", 4);  put32(4, 36 + dataBytes);  memcpy(buf + 8, "WAVE", 4);
  memcpy(buf + 12, "fmt ", 4); put32(16, 16); put16(20, 1); put16(22, 2);
  put32(24, SAMPLE_RATE); put32(28, SAMPLE_RATE * 4); put16(32, 4); put16(34, 16);
  memcpy(buf + 36, "data", 4); put32(40, dataBytes);

  int16_t *pcm = (int16_t *)(buf + 44);
  for (uint32_t n = 0; n < numSamples; n++) {
    float hz = (n < samplesPerTone) ? TONE1_HZ : TONE2_HZ;
    uint32_t k = n % samplesPerTone;
    float envelope = 1.0f - (float)k / samplesPerTone;           // simple fade-out per tone
    int16_t v = (int16_t)(AMPLITUDE * envelope * sinf(2.0f * PI * hz * k / SAMPLE_RATE));
    pcm[2 * n] = v;       // left
    pcm[2 * n + 1] = v;   // right
  }
  *outLen = total;
  return buf;
}

static void goToSleep() {
  rtc_gpio_pullup_en(BUTTON_PIN);
  rtc_gpio_pulldown_dis(BUTTON_PIN);
  esp_sleep_enable_ext0_wakeup(BUTTON_PIN, 0);   // wake when the button pulls GPIO2 LOW
  Serial.println("Going to deep sleep. Press the button to wake. (USB will disconnect.)");
  Serial.flush();
  delay(100);
  esp_deep_sleep_start();
}

void setup() {
  int64_t tAppStart = esp_timer_get_time();     // µs since the app started (excludes ROM/bootloader time)
  wakeCount++;
  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();

  // Audio first — the doorbell's job is to make sound, logging comes after.
  i2s.setPins(I2S_BCLK, I2S_LRC, I2S_DIN);
  bool i2sOk = i2s.begin(I2S_MODE_STD, SAMPLE_RATE, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO);
  int64_t tI2sReady = esp_timer_get_time();

  size_t wavLen = 0;
  uint8_t *wav = buildDingDongWav(&wavLen);
  int64_t tWavBuilt = esp_timer_get_time();

  int64_t tPlayStart = esp_timer_get_time();
  if (i2sOk && wav) i2s.playWAV(wav, wavLen);
  int64_t tPlayDone = esp_timer_get_time();
  free(wav);

  // USB reconnects after a deep-sleep wake, so give the Serial Monitor time to reattach.
  Serial.begin(115200);
  delay(3000);
  Serial.println("\n=== S-01 wake and play ===");
  Serial.printf("Wake #%lu, cause: %s\n", (unsigned long)wakeCount,
                cause == ESP_SLEEP_WAKEUP_EXT0 ? "BUTTON (ext0)" : "power-on/reset");
  Serial.printf("I2S begin: %s | WAV built: %s (%u bytes)\n", i2sOk ? "OK" : "FAILED",
                wav ? "OK" : "FAILED", (unsigned)wavLen);
  Serial.printf("App start -> I2S ready:  %lld ms\n", (tI2sReady - tAppStart) / 1000);
  Serial.printf("App start -> WAV built:  %lld ms\n", (tWavBuilt - tAppStart) / 1000);
  Serial.printf("App start -> play start: %lld ms\n", (tPlayStart - tAppStart) / 1000);
  Serial.printf("Play duration:           %lld ms\n", (tPlayDone - tPlayStart) / 1000);
  Serial.println("NOTE: these times exclude ROM/bootloader time before the app starts.");
  Serial.println("Measure true press-to-sound with the phone-recording procedure.");

  goToSleep();
}

void loop() {}
