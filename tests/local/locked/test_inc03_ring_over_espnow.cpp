// INC-03 acceptance checks (Local tier). Locked: written from the increment card before generation.
// Card: docs/firm/increments/INC-03.md. Protected by A-09; only the Verifier changes these.
// The contract is tested through its master copy (firmware/common/logic/); the gate's COMMON check
// proves each unit's copy is identical to it.
#include "../third_party/doctest.h"
#include "common/logic/contract/ring.h"
#include "doorbell/src/logic/notify/notify.h"

#include <cstdint>
#include <cstdio>
#include <vector>

using namespace contract::ring;

namespace {

// The golden sample of contract ring v1.0 (A-07): ring number 0x1234. Make runs from tests/local.
std::vector<uint8_t> golden() {
    std::vector<uint8_t> bytes;
    FILE* f = std::fopen("golden/ring_1_0.bin", "rb");
    REQUIRE_MESSAGE(f != nullptr, "golden/ring_1_0.bin is missing");
    int c;
    while ((c = std::fgetc(f)) != EOF) bytes.push_back(static_cast<uint8_t>(c));
    std::fclose(f);
    return bytes;
}

struct FakeLink : doorbell::RingLink {
    std::vector<std::vector<uint8_t>> sent;
    bool result = true;
    bool send(const uint8_t* msg, size_t len) override {
        sent.emplace_back(msg, msg + len);
        return result;
    }
};

}  // namespace

TEST_CASE("AC1 [contract] encoding a v1.0 ring gives the golden sample, byte for byte") {
    uint8_t buf[16] = {};
    const size_t n = encode(0x1234, buf, sizeof buf);
    CHECK(n == kSize);
    CHECK(std::vector<uint8_t>(buf, buf + n) == golden());
}

TEST_CASE("AC2 [contract] encode needs kSize bytes of room and writes nothing if it has less") {
    uint8_t small[kSize - 1];
    for (auto& b : small) b = 0xEE;
    CHECK(encode(1, small, sizeof small) == 0);
    for (auto b : small) CHECK(b == 0xEE);
    uint8_t exact[kSize] = {};
    CHECK(encode(1, exact, sizeof exact) == kSize);
}

TEST_CASE("AC3 [contract] the golden sample decodes as a v1.0 ring with its ring number") {
    const auto g = golden();
    Ring r{99, 99};
    CHECK(decode(g.data(), g.size(), r) == Decode::Ok);
    CHECK(r.minor == 0);
    CHECK(r.seq == 0x1234);
}

TEST_CASE("AC4 [contract] a newer minor version with extra bytes at the end is still a ring") {
    const uint8_t msg[] = {'F', 'D', 'B', 1, 3, 1, 0x02, 0x01, 0xAA, 0xBB, 0xCC};
    Ring r{99, 99};
    CHECK(decode(msg, sizeof msg, r) == Decode::Ok);
    CHECK(r.minor == 3);
    CHECK(r.seq == 0x0102);
}

TEST_CASE("AC5 [contract] a different major version fails safe: not read as a ring") {
    Ring r{99, 99};
    const uint8_t v2[] = {'F', 'D', 'B', 2, 0, 1, 0x34, 0x12};
    CHECK(decode(v2, sizeof v2, r) == Decode::WrongMajor);
    const uint8_t v0[] = {'F', 'D', 'B', 0, 9, 1, 0x34, 0x12};
    CHECK(decode(v0, sizeof v0, r) == Decode::WrongMajor);
    const uint8_t v2short[] = {'F', 'D', 'B', 2, 0};  // a v2 message may be shorter than v1
    CHECK(decode(v2short, sizeof v2short, r) == Decode::WrongMajor);
    CHECK(r.seq == 99);  // untouched
}

TEST_CASE("AC6 [contract] other devices' messages aren't ours") {
    Ring r{99, 99};
    const uint8_t spike[] = {'S', '0', '3', ' ', 'R', 'I', 'N', 'G'};  // spike S-03's ring
    CHECK(decode(spike, sizeof spike, r) == Decode::NotOurs);
    const uint8_t two[] = {'F', 'D'};
    CHECK(decode(two, sizeof two, r) == Decode::NotOurs);
    CHECK(decode(nullptr, 0, r) == Decode::NotOurs);
    CHECK(r.seq == 99);
}

TEST_CASE("AC7 [contract] a cut-short message is rejected") {
    const auto g = golden();
    Ring r{99, 99};
    for (size_t len = 3; len < kSize; ++len) {
        CAPTURE(len);
        CHECK(decode(g.data(), len, r) == Decode::TooShort);
    }
    CHECK(r.seq == 99);
}

TEST_CASE("AC8 [contract] a readable message of another type isn't a ring") {
    const uint8_t msg[] = {'F', 'D', 'B', 1, 0, 2, 0x34, 0x12};
    Ring r{99, 99};
    CHECK(decode(msg, sizeof msg, r) == Decode::WrongType);
    CHECK(r.seq == 99);
}

TEST_CASE("AC9 [Doorbell] a button wake sends one v1.0 ring carrying the ring number") {
    FakeLink link;
    uint16_t seq = 0x1234;
    CHECK(doorbell::notifyOnWake(doorbell::WakeReason::Button, seq, link));
    REQUIRE(link.sent.size() == 1);
    CHECK(link.sent[0] == golden());
}

TEST_CASE("AC10 [Doorbell] each ring has the next number, wrapping after 65535") {
    FakeLink link;
    uint16_t seq = 65534;
    for (int i = 0; i < 3; ++i) doorbell::notifyOnWake(doorbell::WakeReason::Button, seq, link);
    REQUIRE(link.sent.size() == 3);
    Ring r{};
    const uint16_t want[3] = {65534, 65535, 0};
    for (int i = 0; i < 3; ++i) {
        CAPTURE(i);
        REQUIRE(decode(link.sent[i].data(), link.sent[i].size(), r) == Decode::Ok);
        CHECK(r.seq == want[i]);
    }
    CHECK(seq == 1);
}

TEST_CASE("AC11 [Doorbell] power-on and other wakes send nothing and use no number") {
    FakeLink link;
    uint16_t seq = 7;
    CHECK_FALSE(doorbell::notifyOnWake(doorbell::WakeReason::PowerOn, seq, link));
    CHECK_FALSE(doorbell::notifyOnWake(doorbell::WakeReason::Other, seq, link));
    CHECK(link.sent.empty());
    CHECK(seq == 7);
    doorbell::notifyOnWake(doorbell::WakeReason::Button, seq, link);  // the next ring uses 7
    REQUIRE(link.sent.size() == 1);
    Ring r{};
    REQUIRE(decode(link.sent[0].data(), link.sent[0].size(), r) == Decode::Ok);
    CHECK(r.seq == 7);
}

TEST_CASE("AC12 [Doorbell] a failed send is reported, and the next ring still gets a new number") {
    FakeLink link;
    link.result = false;
    uint16_t seq = 40;
    CHECK_FALSE(doorbell::notifyOnWake(doorbell::WakeReason::Button, seq, link));
    REQUIRE(link.sent.size() == 1);
    CHECK(seq == 41);
}
