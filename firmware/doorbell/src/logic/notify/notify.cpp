// Telling the Indoor Ringer: one ring message per button wake (contract ring v1.0).
#include "notify.h"
#include "../common/contract/ring.h"

namespace doorbell {

bool notifyOnWake(WakeReason reason, uint16_t& seq, RingLink& link) {
    if (reason != WakeReason::Button) return false;  // power-on and other wakes: radio stays off
    uint8_t msg[contract::ring::kSize];
    const size_t len = contract::ring::encode(seq, msg, sizeof msg);
    ++seq;  // wraps 65535 -> 0 (uint16_t); advanced whether or not the send works
    return len != 0 && link.send(msg, len);
}

}  // namespace doorbell
