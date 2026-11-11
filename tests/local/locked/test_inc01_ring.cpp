// INC-01 acceptance checks (Local tier). Locked: written from the increment card before generation.
// Card: docs/firm/increments/INC-01.md. Protected by A-09; only the Verifier changes these.
#include "../third_party/doctest.h"
#include "doorbell/src/logic/ring/ring.h"

#include <string>
#include <vector>

using namespace doorbell;

namespace {

const uint8_t kBytes[4] = {1, 2, 3, 4};
const Sound kBuiltin{kBytes, sizeof kBytes};

// Every action the logic takes on the hardware, in order.
struct Log {
    std::vector<std::string> events;
    const uint8_t* playedData = nullptr;
    size_t playedLen = 0;
    uint32_t waitedMs = 0;
};

struct FakeAudio : AudioOut {
    Log& log;
    bool result = true;
    explicit FakeAudio(Log& l) : log(l) {}
    bool play(const Sound& s) override {
        log.events.push_back("play");
        log.playedData = s.data;
        log.playedLen = s.len;
        return result;
    }
};

struct FakePower : PowerCtl {
    Log& log;
    WakeReason reason = WakeReason::Button;
    uint32_t heldForMs = 0;  // button stays held this long after the sound starts; UINT32_MAX = stuck
    explicit FakePower(Log& l) : log(l) {}
    WakeReason wakeReason() override { return reason; }
    bool buttonHeld() override { return log.waitedMs < heldForMs; }
    void waitMs(uint32_t ms) override {
        log.events.push_back("wait");
        log.waitedMs += ms;
    }
    void sleepUntilButton() override { log.events.push_back("sleep"); }
};

int count(const Log& log, const std::string& what) {
    int n = 0;
    for (const auto& e : log.events) n += (e == what);
    return n;
}

}  // namespace

TEST_CASE("INC-01 AC1: a button wake plays the built-in sound first, once, then sleeps") {
    Log log; FakeAudio audio(log); FakePower power(log);
    power.reason = WakeReason::Button;
    onWake(kBuiltin, audio, power);
    REQUIRE(log.events.size() == 2);
    CHECK(log.events[0] == "play");
    CHECK(log.events[1] == "sleep");
}

TEST_CASE("INC-01 AC2: the sound played is the built-in sound it was given") {
    Log log; FakeAudio audio(log); FakePower power(log);
    onWake(kBuiltin, audio, power);
    CHECK(log.playedData == kBytes);
    CHECK(log.playedLen == sizeof kBytes);
}

TEST_CASE("INC-01 AC3: a power-on wake also plays the built-in sound once, then sleeps") {
    Log log; FakeAudio audio(log); FakePower power(log);
    power.reason = WakeReason::PowerOn;
    onWake(kBuiltin, audio, power);
    REQUIRE(log.events.size() == 2);
    CHECK(log.events[0] == "play");
    CHECK(log.events[1] == "sleep");
}

TEST_CASE("INC-01 AC4: any other wake goes back to sleep without a sound") {
    Log log; FakeAudio audio(log); FakePower power(log);
    power.reason = WakeReason::Other;
    onWake(kBuiltin, audio, power);
    CHECK(count(log, "play") == 0);
    REQUIRE(log.events.size() == 1);
    CHECK(log.events[0] == "sleep");
}

TEST_CASE("INC-01 AC5: if playback fails, it still goes to sleep") {
    Log log; FakeAudio audio(log); FakePower power(log);
    audio.result = false;
    onWake(kBuiltin, audio, power);
    CHECK(count(log, "play") == 1);
    REQUIRE(!log.events.empty());
    CHECK(log.events.back() == "sleep");
    CHECK(count(log, "sleep") == 1);
}

TEST_CASE("INC-01 AC6: a held button gives one sound; it sleeps only after release") {
    Log log; FakeAudio audio(log); FakePower power(log);
    power.heldForMs = 1500;
    onWake(kBuiltin, audio, power);
    REQUIRE(!log.events.empty());
    CHECK(log.events.front() == "play");          // sound first, even with the button down
    CHECK(count(log, "play") == 1);
    CHECK(log.events.back() == "sleep");
    CHECK(count(log, "sleep") == 1);
    CHECK(log.waitedMs >= 1500);                  // didn't sleep while still held
    CHECK(log.waitedMs < 1500 + kReleasePollMs + 1);
}

TEST_CASE("INC-01 AC7: a stuck button gives up after the time limit and sleeps") {
    Log log; FakeAudio audio(log); FakePower power(log);
    power.heldForMs = UINT32_MAX;
    onWake(kBuiltin, audio, power);
    CHECK(count(log, "play") == 1);
    REQUIRE(!log.events.empty());
    CHECK(log.events.back() == "sleep");
    CHECK(log.waitedMs >= kReleaseTimeoutMs);
    CHECK(log.waitedMs < kReleaseTimeoutMs + kReleasePollMs + 1);
}
