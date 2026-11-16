// Telling the Indoor Ringer (backlog rank 3). NOT the ring path: the sound never waits for this (A-05).
// The hardware layer runs it on the other processor core once the chime has started (DL-036).
// Declared in the INC-03 lock commit; behavior is defined by tests/local/locked/test_inc03_ring_over_espnow.cpp.
#pragma once
#include "../ports.h"

namespace doorbell {

// Runs once per wake. Only a button wake rings the Ringer: it sends one ring message (contract ring
// v1.0) carrying `seq`, then advances `seq` by one (wrapping 65535 -> 0) whether or not the send
// worked. A power-on or any other wake sends nothing and leaves `seq` alone, so the radio stays off.
// Returns true only if a message was sent and the link accepted it.
bool notifyOnWake(WakeReason reason, uint16_t& seq, RingLink& link);

}  // namespace doorbell
