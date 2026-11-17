// Helper test (unlocked), written during INC-03 generation for DL-038, which was decided after the
// lock: which decoded messages ring the Indoor Ringer. The Verifier may lock it later.
#include "../third_party/doctest.h"
#include "indoor-ringer/src/logic/listen/listen.h"

using contract::ring::Decode;

TEST_CASE("helper [Ringer] a readable ring and a version mismatch ring; nothing else does (DL-038)") {
    CHECK(ringer::ringsFor(Decode::Ok));
    CHECK(ringer::ringsFor(Decode::WrongMajor));
    CHECK_FALSE(ringer::ringsFor(Decode::NotOurs));
    CHECK_FALSE(ringer::ringsFor(Decode::TooShort));
    CHECK_FALSE(ringer::ringsFor(Decode::WrongType));
}
