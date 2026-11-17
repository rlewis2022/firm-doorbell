#include "espnow_link.h"
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "i2s_audio.h"

namespace doorbell {

namespace {
const uint8_t kBroadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
SemaphoreHandle_t soundStarted = nullptr;
SemaphoreHandle_t jobDone = nullptr;
void (*afterSoundJob)() = nullptr;
uint32_t afterSoundTimeoutMs = 0;

void signalSoundStarted() {
    if (soundStarted) xSemaphoreGive(soundStarted);
}

void afterSoundTask(void*) {
    xSemaphoreTake(soundStarted, pdMS_TO_TICKS(afterSoundTimeoutMs));
    afterSoundJob();
    xSemaphoreGive(jobDone);
    vTaskDelete(nullptr);
}
}  // namespace

bool EspNowLink::start() {
    WiFi.mode(WIFI_STA);  // station mode for ESP-NOW only; never joins a network (A-03)
    esp_wifi_set_channel(channel_, WIFI_SECOND_CHAN_NONE);
    if (esp_now_init() != ESP_OK) return false;
    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, kBroadcast, sizeof kBroadcast);
    peer.channel = 0;  // the current channel
    peer.ifidx = WIFI_IF_STA;
    peer.encrypt = false;  // broadcast, unencrypted for now (DL-037; rank 13)
    return esp_now_add_peer(&peer) == ESP_OK;
}

bool EspNowLink::send(const uint8_t* msg, size_t len) {
    if (!started_) started_ = start();
    if (!started_) return false;
    return esp_now_send(kBroadcast, msg, len) == ESP_OK;
}

void runAfterSoundStarts(I2sAudio& audio, void (*job)(), uint32_t timeoutMs) {
    soundStarted = xSemaphoreCreateBinary();
    jobDone = xSemaphoreCreateBinary();
    afterSoundJob = job;
    afterSoundTimeoutMs = timeoutMs;
    audio.onStart(signalSoundStarted);
    // Core 0; the chime plays on core 1 (Arduino's loop core).
    xTaskCreatePinnedToCore(afterSoundTask, "ring-link", 6144, nullptr, 1, nullptr, 0);
}

void waitForAfterSoundJob(uint32_t maxMs) {
    if (jobDone && xSemaphoreTake(jobDone, pdMS_TO_TICKS(maxMs)) == pdTRUE) xSemaphoreGive(jobDone);
}

}  // namespace doorbell
