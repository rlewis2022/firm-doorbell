// SPIKE S-03 — Doorbell side (v3: stays awake 20 s after the chime, repeating its table; reports channel and MAC). THROWAWAY CODE: never copied into the product (spikes/README.md).
// Question: can the Doorbell start its radio and send a message to the Indoor Ringer while the chime
// plays, without delaying or disturbing it, and what does a check-in cost in time and current?
//
// Same wiring as INC-01: MAX98357A BCLK=GPIO4, LRC=GPIO5, DIN=GPIO6; button GPIO2 to GND.
// Board settings: ESP32S3 Dev Module, USB CDC On Boot Enabled, Flash 4MB, PSRAM QSPI, No OTA, core 3.3.12.
//
// Pick ONE mode, upload, then press the button a few times (see README.md):
//   0 = sound only (baseline: no radio at all)
//   1 = radio in PARALLEL: radio work runs on core 0 while the chime plays on core 1
//   2 = radio AFTER the chime (to compare)
//   3 = CURRENT: after the chime, radio on and idle for 15 s, then radio off and awake for 15 s,
//       then asleep (read your meter in each phase)
#define MODE 1

#include <ESP_I2S.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "driver/rtc_io.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "builtin_dingdong.h"

static const gpio_num_t BUTTON = GPIO_NUM_2;
static const uint8_t BROADCAST[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static const uint8_t CHANNEL = 1;

I2SClass i2s;

// Timestamps in microseconds since boot (esp_timer). 0 = didn't happen.
struct Times {
  int64_t appStart, audioStart, audioEnd;
  int64_t radioStart, wifiUp, espnowReady, ringSent, checkSent, replyAt;
  int sendRingErr, sendCheckErr;
  bool gotReply;
};
static Times T;
RTC_DATA_ATTR Times lastWake;  // kept through deep sleep, printed again on the next wake
RTC_DATA_ATTR int wakeCount = 0;

static volatile int64_t replyAtUs = 0;
static volatile bool radioDone = false;

static void onRecv(const esp_now_recv_info_t* info, const uint8_t* data, int len) {
  (void)info;
  if (len >= 11 && memcmp(data, "S03 NOTHING", 11) == 0 && replyAtUs == 0) replyAtUs = esp_timer_get_time();
}

static void radioWork() {
  T.radioStart = esp_timer_get_time();
  WiFi.mode(WIFI_STA);  // station mode for ESP-NOW only; never joins a network (A-03)
  esp_wifi_set_channel(CHANNEL, WIFI_SECOND_CHAN_NONE);
  T.wifiUp = esp_timer_get_time();
  if (esp_now_init() != ESP_OK) return;
  esp_now_register_recv_cb(onRecv);
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, BROADCAST, 6);
  peer.channel = 0;  // current channel
  peer.ifidx = WIFI_IF_STA;
  peer.encrypt = false;
  esp_now_add_peer(&peer);
  T.espnowReady = esp_timer_get_time();
  T.sendRingErr = esp_now_send(BROADCAST, (const uint8_t*)"S03 RING", 8);
  T.ringSent = esp_timer_get_time();
  T.sendCheckErr = esp_now_send(BROADCAST, (const uint8_t*)"S03 CHECK?", 10);
  T.checkSent = esp_timer_get_time();
  const int64_t deadline = T.checkSent + 300000;  // wait up to 300 ms for "nothing new"
  while (replyAtUs == 0 && esp_timer_get_time() < deadline) delay(1);
  T.gotReply = replyAtUs != 0;
  T.replyAt = replyAtUs;
}

static void radioTask(void*) {
  radioWork();
  radioDone = true;
  vTaskDelete(nullptr);
}

static void playChime() {
  T.audioStart = esp_timer_get_time();
  i2s.setPins(4, 5, 6);
  i2s.begin(I2S_MODE_STD, 24000, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO);
  i2s.playWAV(const_cast<uint8_t*>(builtin_dingdong_wav), builtin_dingdong_wav_len);  // blocks ~2.9 s
  T.audioEnd = esp_timer_get_time();
}

static void ms(const char* label, int64_t from, int64_t to) {
  if (from && to) Serial.printf("  %-34s %8.1f ms\n", label, (to - from) / 1000.0);
  else Serial.printf("  %-34s %8s\n", label, "-");
}

static void printTimes(const Times& t, const char* title) {
  Serial.printf("%s\n", title);
  ms("boot -> app start", 1, t.appStart);
  ms("app start -> audio start", t.appStart, t.audioStart);
  ms("chime length (audio start -> end)", t.audioStart, t.audioEnd);
  ms("app start -> radio start", t.appStart, t.radioStart);
  ms("radio: WiFi.mode(STA) + channel", t.radioStart, t.wifiUp);
  ms("radio: ESP-NOW init + peer", t.wifiUp, t.espnowReady);
  ms("radio start -> ring sent", t.radioStart, t.ringSent);
  ms("app start -> ring sent", t.appStart, t.ringSent);
  ms("check-in: request -> reply", t.checkSent, t.replyAt);
  if (t.radioStart) {
    uint8_t ch = 0; wifi_second_chan_t sec; esp_wifi_get_channel(&ch, &sec);
    Serial.printf("  radio: channel %d, my MAC %s\n", ch, WiFi.macAddress().c_str());
  }
  if (t.radioStart) Serial.printf("  send results: ring=%d check=%d (0 = OK)  reply: %s\n",
                                  t.sendRingErr, t.sendCheckErr, t.gotReply ? "yes" : "NO (timed out)");
}

static void sleepNow() {
  rtc_gpio_deinit(BUTTON);  // after an ext0 wake the pin is still under RTC control
  pinMode(BUTTON, INPUT_PULLUP);
  while (digitalRead(BUTTON) == LOW) delay(10);  // don't sleep while the button is held
  rtc_gpio_pullup_en(BUTTON);
  rtc_gpio_pulldown_dis(BUTTON);
  esp_sleep_enable_ext0_wakeup(BUTTON, 0);
  esp_deep_sleep_start();
}

void setup() {
  T = {};
  T.appStart = esp_timer_get_time();
  ++wakeCount;
#if MODE == 1
  xTaskCreatePinnedToCore(radioTask, "radio", 6144, nullptr, 1, nullptr, 0);  // core 0; chime on core 1
#endif
  playChime();  // the sound starts first in every mode
#if MODE == 2
  radioWork();
#endif
#if MODE == 1
  const int64_t wait = esp_timer_get_time() + 1000000;
  while (!radioDone && esp_timer_get_time() < wait) delay(1);
#endif

  Serial.begin(115200);
  // Stay awake 20 s and repeat the table every 2 s, so the Serial Monitor has time to reconnect
  // after the wake (the USB port disappears while the board sleeps).
  const Times prev = lastWake;
  lastWake = T;
  for (int i = 0; i < 10; ++i) {
    delay(2000);
    Serial.printf("\n=== S-03 Doorbell v3, MODE %d, wake #%d, cause %d (repeat %d of 10) ===\n", MODE,
                  wakeCount, (int)esp_sleep_get_wakeup_cause(), i + 1);
    printTimes(T, "This wake:");
    if (wakeCount > 1) printTimes(prev, "Previous wake:");
  }

#if MODE == 3
  WiFi.mode(WIFI_STA);
  esp_wifi_set_channel(CHANNEL, WIFI_SECOND_CHAN_NONE);
  esp_now_init();
  Serial.println("CURRENT phase A: awake, radio ON (idle) for 15 s -> read the meter now");
  delay(15000);
  esp_now_deinit();
  WiFi.mode(WIFI_OFF);
  Serial.println("CURRENT phase B: awake, radio OFF for 15 s -> read the meter now");
  delay(15000);
  Serial.println("CURRENT phase C: going to deep sleep -> read the meter (amp still powered)");
  delay(100);
#endif
  Serial.println("Going to deep sleep. Press the button to wake.");
  Serial.flush();
  sleepNow();
}

void loop() {}
