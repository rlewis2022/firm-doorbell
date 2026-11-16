#include "listen.h"

namespace ringer {

bool ringsFor(contract::ring::Decode result) {
    using contract::ring::Decode;
    return result == Decode::Ok || result == Decode::WrongMajor;
}

}  // namespace ringer
