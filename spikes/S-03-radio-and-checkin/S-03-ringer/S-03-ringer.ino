// SPIKE S-03 — Indoor Ringer side (v2: reports every packet, its channel, and keeps printing its status). THROWAWAY CODE: never copied into the product (spikes/README.md).
// Listens for the Doorbell's spike messages over ESP-NOW:
//   "S03 RING"   -> plays a short 1 kHz beep (so the phone recording shows when the ring arrived)
//   "S03 CHECK?" -> answers "S03 NOTHING" straight away (the check-in round trip)
// and prints each arrival over serial.
//
// Same wiring as INC-02: MAX98357A BCLK=GPIO4, LRC=GPIO5, DIN=GPIO6, GAIN 100 kOhm to GND.
// Board settings: ESP32S3 Dev Module, USB CDC On Boot Enabled, Flash 4MB, PSRAM QSPI, No OTA, core 3.3.12.

#include <ESP_I2S.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "esp_timer.h"

static const uint8_t BROADCAST[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static const uint8_t CHANNEL = 1;
static const int RATE = 24000;
static const int BEEP_MS = 120;

I2SClass i2s;
static uint8_t beepWav[44 + RATE * BEEP_MS / 1000 * 2];
static TaskHandle_t beepTask = nullptr;

static volatile int ringCount = 0, checkCount = 0, anyCount = 0, lastLen = 0, lastRssi = 0;
static volatile uint8_t lastFirst[12];
static volatile uint8_t lastSrc[6];
static bool espnowOk = false;
static volatile int64_t lastRingUs = 0, lastCheckUs = 0;

static void put32(uint8_t* p, uint32_t v) { for (int i = 0; i < 4; ++i) p[i] = (v >> (8 * i)) & 0xFF; }
static void put16(uint8_t* p, uint16_t v) { p[0] = v & 0xFF; p[1] = v >> 8; }

static void makeBeep() {  // a 1 kHz, 120 ms mono 16-bit WAV, built once at start-up (spike only)
  const uint32_t n = RATE * BEEP_MS / 1000, data = n * 2;
  memcpy(beepWav, "RIFF", 4); put32(beepWav + 4, 36 + data); memcpy(beepWav + 8, "WAVEfmt ", 8);
  put32(beepWav + 16, 16); put16(beepWav + 20, 1); put16(beepWav + 22, 1); put32(beepWav + 24, RATE);
  put32(beepWav + 28, RATE * 2); put16(beepWav + 32, 2); put16(beepWav + 34, 16);
  memcpy(beepWav + 36, "data", 4); put32(beepWav + 40, data);
  for (uint32_t i = 0; i < n; ++i) {
    const float env = (i < 48) ? i / 48.0f : (i > n - 48 ? (n - i) / 48.0f : 1.0f);
    put16(beepWav + 44 + 2 * i, (uint16_t)(int16_t)(12000 * env * sinf(2 * PI * 1000.0f * i / RATE)));
  }
}

static void beeper(void*) {
  for (;;) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    i2s.playWAV(beepWav, sizeof beepWav);
  }
}

static void onRecv(const esp_now_recv_info_t* info, const uint8_t* data, int len) {
  const int64_t now = esp_timer_get_time();
  ++anyCount; lastLen = len; lastRssi = info->rx_ctrl ? info->rx_ctrl->rssi : 0;
  for (int i = 0; i < 12; ++i) lastFirst[i] = i < len ? data[i] : 0;
  for (int i = 0; i < 6; ++i) lastSrc[i] = info->src_addr[i];
  if (len >= 8 && memcmp(data, "S03 RING", 8) == 0) {
    lastRingUs = now; ++ringCount;
    if (beepTask) xTaskNotifyGive(beepTask);
  } else if (len >= 10 && memcmp(data, "S03 CHECK?", 10) == 0) {
    lastCheckUs = now; ++checkCount;
    if (!esp_now_is_peer_exist(info->src_addr)) {
      esp_now_peer_info_t peer = {};
      memcpy(peer.peer_addr, info->src_addr, 6);
      peer.channel = 0;
      peer.ifidx = WIFI_IF_STA;
      peer.encrypt = false;
      esp_now_add_peer(&peer);
    }
    esp_now_send(info->src_addr, (const uint8_t*)"S03 NOTHING", 11);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  makeBeep();
  i2s.setPins(4, 5, 6);
  i2s.begin(I2S_MODE_STD, RATE, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO);
  xTaskCreatePinnedToCore(beeper, "beep", 4096, nullptr, 1, &beepTask, 1);

  WiFi.mode(WIFI_STA);  // ESP-NOW only for this spike; doesn't join a network
  esp_wifi_set_channel(CHANNEL, WIFI_SECOND_CHAN_NONE);
  espnowOk = (esp_now_init() == ESP_OK);
  esp_now_register_recv_cb(onRecv);
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, BROADCAST, 6);
  peer.ifidx = WIFI_IF_STA;
  esp_now_add_peer(&peer);
  uint8_t ch = 0; wifi_second_chan_t sec; esp_wifi_get_channel(&ch, &sec);
  Serial.printf("S-03 Ringer v2 listening. ESP-NOW %s, channel %d. My MAC: %s\n", espnowOk ? "OK" : "INIT FAILED", ch,
                WiFi.macAddress().c_str());
  xTaskNotifyGive(beepTask);  // one beep at start-up: the Ringer is ready
}

void loop() {
  static int seenRings = 0, seenChecks = 0, seenAny = 0;
  static uint32_t lastStatus = 0;
  if (millis() - lastStatus > 5000) {  // a status line every 5 s, so an open monitor always shows something
    lastStatus = millis();
    uint8_t ch = 0; wifi_second_chan_t sec; esp_wifi_get_channel(&ch, &sec);
    Serial.printf("[status] ESP-NOW %s, channel %d, MAC %s, packets received %d\n", espnowOk ? "OK" : "INIT FAILED",
                  ch, WiFi.macAddress().c_str(), anyCount);
  }
  if (anyCount != seenAny) {
    seenAny = anyCount;
    Serial.printf("[packet] #%d from %02X:%02X:%02X:%02X:%02X:%02X, %d bytes, RSSI %d dBm, starts \"%.12s\"\n", seenAny,
                  lastSrc[0], lastSrc[1], lastSrc[2], lastSrc[3], lastSrc[4], lastSrc[5], lastLen, lastRssi,
                  (const char*)lastFirst);
  }
  if (ringCount != seenRings) {
    seenRings = ringCount;
    Serial.printf("[%10.3f s] RING #%d received -> beep\n", lastRingUs / 1e6, seenRings);
  }
  if (checkCount != seenChecks) {
    seenChecks = checkCount;
    Serial.printf("[%10.3f s] CHECK? #%d received -> answered \"nothing new\"\n", lastCheckUs / 1e6, seenChecks);
  }
  delay(5);
}
