// LOCK-COMMIT STUB (INC-01). Empty on purpose, so the locked checks compile, run and fail on their
// assertions before generation. Generation replaces this body.
#include "ring.h"

namespace doorbell {

void onWake(const Sound& builtin, AudioOut& audio, PowerCtl& power) {
    (void)builtin;
    (void)audio;
    (void)power;
}

}  // namespace doorbell
