#include "notify.h"
#include "../common/contract/ring.h"

namespace doorbell {

bool notifyOnWake(WakeReason reason, uint16_t& seq, RingLink& link) {
    (void)reason; (void)seq; (void)link;
    return false;  // LOCK-COMMIT STUB: generation replaces this body
}

}  // namespace doorbell
