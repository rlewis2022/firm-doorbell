// Which received messages ring the Indoor Ringer (contract ring, A-07; DL-038).
// Standard headers and logic only (A-02). Not the ring path: the hardware layer calls it when a
// message arrives and passes rings on to the Ringer through Inputs::takeRing().
#pragma once
#include "../common/contract/ring.h"

namespace ringer {

// A readable ring rings. On a version mismatch (another major version) it rings too: for a doorbell,
// failing safe means ringing (DL-038). Messages that aren't ours, are cut short, or are another
// type don't.
bool ringsFor(contract::ring::Decode result);

}  // namespace ringer
