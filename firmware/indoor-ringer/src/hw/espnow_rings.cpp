#include "espnow_rings.h"
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "../logic/common/contract/ring.h"
#include "../logic/listen/listen.h"

namespace ringer {

namespace {
struct Heard {
    uint32_t atMs;
    contract::ring::Decode result;
    uint8_t major;
    uint16_t seq;
};
constexpr int kQueue = 8;
Heard queue[kQueue];
int head = 0, count = 0;
uint32_t ignored = 0, ignoredReported = 0;
contract::ring::Decode lastIgnored = contract::ring::Decode::NotOurs;
portMUX_TYPE queueLock = portMUX_INITIALIZER_UNLOCKED;

const char* name(contract::ring::Decode d) {
    switch (d) {
        case contract::ring::Decode::Ok:         return "ok";
        case contract::ring::Decode::NotOurs:    return "not ours";
        case contract::ring::Decode::WrongMajor: return "version mismatch";
        case contract::ring::Decode::TooShort:   return "cut short";
        case contract::ring::Decode::WrongType:  return "not a ring";
    }
    return "?";
}

// Runs in the Wi-Fi task: keep it short. No printing here.
void onReceive(const esp_now_recv_info_t* info, const uint8_t* data, int len) {
    (void)info;
    const uint32_t now = millis();
    contract::ring::Ring ring{0, 0};
    const contract::ring::Decode result =
        contract::ring::decode(data, len > 0 ? static_cast<size_t>(len) : 0, ring);
    portENTER_CRITICAL(&queueLock);
    if (ringsFor(result)) {
        if (count < kQueue) {
            queue[(head + count) % kQueue] = {now, result, len > 3 ? data[3] : uint8_t(0), ring.seq};
            ++count;
        }
    } else {
        ++ignored;
        lastIgnored = result;
    }
    portEXIT_CRITICAL(&queueLock);
}
}  // namespace

bool EspNowRings::begin(uint8_t channel) {
    WiFi.mode(WIFI_STA);  // ESP-NOW only for now; joining Wi-Fi comes with rank 7
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
    const bool ok = esp_now_init() == ESP_OK && esp_now_register_recv_cb(onReceive) == ESP_OK;
    Serial.printf("Indoor Ringer: %s for rings on ESP-NOW channel %u (contract ring v%u.%u)\n",
                  ok ? "listening" : "ESP-NOW FAILED TO START, not listening", channel,
                  contract::ring::kMajor, contract::ring::kMinor);
    return ok;
}

bool EspNowRings::take(uint32_t& atMs) {
    Heard h{};
    bool got = false;
    uint32_t newIgnored = 0;
    contract::ring::Decode why = contract::ring::Decode::NotOurs;
    portENTER_CRITICAL(&queueLock);
    if (count > 0) {
        h = queue[head];
        head = (head + 1) % kQueue;
        --count;
        got = true;
    }
    newIgnored = ignored - ignoredReported;
    ignoredReported = ignored;
    why = lastIgnored;
    portEXIT_CRITICAL(&queueLock);
    if (newIgnored) Serial.printf("ignored %lu message(s), last: %s\n", (unsigned long)newIgnored, name(why));
    if (!got) return false;
    if (h.result == contract::ring::Decode::Ok) {
        Serial.printf("ring #%u at %lu ms\n", h.seq, (unsigned long)h.atMs);
    } else {
        Serial.printf("ring (%s: major %u) at %lu ms\n", name(h.result), h.major, (unsigned long)h.atMs);
    }
    atMs = h.atMs;
    return true;
}

}  // namespace ringer
