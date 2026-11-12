// LOCK-COMMIT STUB (INC-02). Empty on purpose, so the locked checks compile, run and fail on their
// assertions before generation. Generation replaces these bodies.
#include "ringer.h"

namespace ringer {

Ringer::Ringer(const Sound& builtin, AudioOut& audio, Inputs& inputs)
    : builtin_(builtin), audio_(audio), in_(inputs) {}

void Ringer::start() {}

void Ringer::step() {}

void Ringer::ring() {}

}  // namespace ringer
