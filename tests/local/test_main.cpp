// Local tier (FIRM): PC-side test runner for logic code. Runs on every iteration in the machine gate.
// Locked acceptance checks and the regression suite live next to this file (protected by A-09).
// Unlocked, AI-drafted helper tests go in tests/local/unlocked/ until the Verifier locks them.
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "third_party/doctest.h"

TEST_CASE("baseline 0: the Local-tier runner builds and runs") {
    CHECK(1 + 1 == 2);
}
