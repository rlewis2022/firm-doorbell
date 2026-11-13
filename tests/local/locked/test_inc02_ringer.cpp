// INC-02 acceptance checks (Local tier). Locked: written from the increment card before generation.
// Card: docs/firm/increments/INC-02.md. Protected by A-09; only the Verifier changes these.
#include "../third_party/doctest.h"
#include "indoor-ringer/src/logic/ring/ringer.h"

#include <cstdint>
#include <vector>

using namespace ringer;

namespace {

const uint8_t kBytes[4] = {9, 8, 7, 6};
const Sound kBuiltin{kBytes, sizeof kBytes};
constexpr uint32_t kChimeMs = 2900;  // the built-in sound's length; play() blocks this long

// A fake world with its own clock. play() advances the clock by kChimeMs, like the real blocking playWAV().
struct World : AudioOut, Inputs {
    uint32_t now = 1000;
    bool down = false;
    std::vector<uint32_t> rings;      // arrival times of ring messages not yet taken
    int plays = 0;
    const uint8_t* playedData = nullptr;
    size_t playedLen = 0;
    bool playResult = true;

    bool play(const Sound& s) override {
        ++plays;
        playedData = s.data;
        playedLen = s.len;
        now += kChimeMs;
        return playResult;
    }
    uint32_t nowMs() override { return now; }
    bool buttonDown() override { return down; }
    bool takeRing(uint32_t& atMs) override {
        if (rings.empty()) return false;
        atMs = rings.front();
        rings.erase(rings.begin());
        return true;
    }
};

// Runs step() every 1 ms for `ms` milliseconds.
void runFor(Ringer& r, World& w, uint32_t ms) {
    const uint32_t end = w.now + ms;
    while (w.now < end) {
        r.step();
        w.now += 1;
    }
}

// A clean press: down for `holdMs`, then up, then idle long enough to settle.
void press(Ringer& r, World& w, uint32_t holdMs = 200) {
    w.down = true;
    runFor(r, w, holdMs);
    w.down = false;
    runFor(r, w, 300);
}

}  // namespace

TEST_CASE("INC-02 AC1: power-on plays the built-in sound once") {
    World w; Ringer r(kBuiltin, w, w);
    r.start();
    CHECK(w.plays == 1);
    runFor(r, w, 2000);
    CHECK(w.plays == 1);
}

TEST_CASE("INC-02 AC2: the sound played is the built-in sound it was given") {
    World w; Ringer r(kBuiltin, w, w);
    r.start();
    CHECK(w.playedData == kBytes);
    CHECK(w.playedLen == sizeof kBytes);
}

TEST_CASE("INC-02 AC3: each press of the test button plays the built-in sound once") {
    World w; Ringer r(kBuiltin, w, w);
    r.start();
    for (int i = 0; i < 5; ++i) press(r, w);
    CHECK(w.plays == 1 + 5);
}

TEST_CASE("INC-02 AC4: holding the test button gives one sound, not a repeat") {
    World w; Ringer r(kBuiltin, w, w);
    r.start();
    press(r, w, 6000);  // held for longer than two chimes
    CHECK(w.plays == 1 + 1);
}

TEST_CASE("INC-02 AC5: contact bounce on press and release counts as one press") {
    World w; Ringer r(kBuiltin, w, w);
    r.start();
    // press with bounce: down/up flickers every 3 ms, then held
    for (int i = 0; i < 4; ++i) { w.down = true; runFor(r, w, 3); w.down = false; runFor(r, w, 3); }
    w.down = true; runFor(r, w, 400);
    // release with bounce, then idle
    for (int i = 0; i < 4; ++i) { w.down = false; runFor(r, w, 3); w.down = true; runFor(r, w, 3); }
    w.down = false; runFor(r, w, 400);
    CHECK(w.plays == 1 + 1);
}

TEST_CASE("INC-02 AC6: a glitch shorter than the debounce time is not a press") {
    World w; Ringer r(kBuiltin, w, w);
    r.start();
    w.down = true; runFor(r, w, kDebounceMs / 3);
    w.down = false; runFor(r, w, 500);
    CHECK(w.plays == 1);
}

TEST_CASE("INC-02 AC7: a ring message plays the built-in sound once") {
    World w; Ringer r(kBuiltin, w, w);
    r.start();
    runFor(r, w, 500);
    w.rings.push_back(w.now);
    runFor(r, w, 500);
    CHECK(w.plays == 1 + 1);
}

TEST_CASE("INC-02 AC8: a ring that arrived while a sound was playing is ignored") {
    World w; Ringer r(kBuiltin, w, w);
    const uint32_t chimeStart = w.now;
    r.start();                                  // plays; the clock is now past the chime
    w.rings.push_back(chimeStart + 1000);       // arrived during that chime
    runFor(r, w, 1000);
    CHECK(w.plays == 1);
    w.rings.push_back(w.now);                   // a ring after the chime still plays
    runFor(r, w, 500);
    CHECK(w.plays == 2);
}

TEST_CASE("INC-02 AC9: if playback fails, the Ringer keeps working") {
    World w; Ringer r(kBuiltin, w, w);
    w.playResult = false;
    r.start();
    press(r, w);
    w.rings.push_back(w.now);
    runFor(r, w, 500);
    CHECK(w.plays == 3);
}
