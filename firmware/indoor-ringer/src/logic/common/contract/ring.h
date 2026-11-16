// Contract: the ring message, Doorbell -> Indoor Ringer over ESP-NOW (absolute A-07; DL-037).
// MASTER COPY. Edit only this file, then run `python3 tools/sync_common.py` to copy it into each unit.
// Standard headers only (A-02). Behavior is defined by tests/local/locked/test_inc03_ring_over_espnow.cpp
// and the golden sample tests/local/golden/ring_1_0.bin.
//
// Version 1.0, 8 bytes, little-endian:
//   byte 0-2  'F' 'D' 'B'   marks the message as ours
//   byte 3    major         a different major can't be read: the receiver ignores it (fails safe)
//   byte 4    minor         a newer minor only adds bytes at the end: the receiver reads what it knows
//   byte 5    type          1 = ring
//   byte 6-7  seq           ring number, counts up by one per ring and wraps (65535 -> 0)
#pragma once
#include <stddef.h>
#include <stdint.h>

namespace contract {
namespace ring {

constexpr uint8_t kMajor = 1;
constexpr uint8_t kMinor = 0;
constexpr uint8_t kTypeRing = 1;
constexpr size_t kSize = 8;  // length of a version 1.0 message

enum class Decode {
    Ok,          // a ring this receiver understands
    NotOurs,     // not one of our messages (fewer than 3 bytes, or other first bytes)
    WrongMajor,  // ours, but a major version this receiver can't read
    TooShort,    // ours and the right major, but cut short
    WrongType    // ours and readable, but not a ring
};

struct Ring {
    uint8_t minor;  // the sender's minor version
    uint16_t seq;
};

// Writes a version 1.0 ring message with this ring number. Returns the bytes written (kSize),
// or 0 (writing nothing) if `cap` is smaller than kSize.
inline size_t encode(uint16_t seq, uint8_t* out, size_t cap) {
    (void)seq; (void)out; (void)cap;
    return 0;  // LOCK-COMMIT STUB: generation replaces this body
}

// Reads a received message. Fills `out` only when it returns Decode::Ok.
// Order: NotOurs, then WrongMajor, then TooShort, then WrongType.
inline Decode decode(const uint8_t* data, size_t len, Ring& out) {
    (void)data; (void)len;
    out = Ring{0, 0};
    return Decode::Ok;  // LOCK-COMMIT STUB: generation replaces this body
}

}  // namespace ring
}  // namespace contract
