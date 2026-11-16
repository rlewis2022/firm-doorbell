# Increment card INC-03: the Doorbell rings the Indoor Ringer over ESP-NOW

- **Backlog item:** rank 3 (Decide §5): the Doorbell sends a ring message over ESP-NOW (contract v1.0);
  the Indoor Ringer rings. Decisions: DL-036 (radio on the other core, in parallel with the chime),
  DL-037 (ring message only; broadcast; shared code in `firmware/common/`; RT-01 rerun).
- **Requirement:** when the Doorbell's button wakes it, the Doorbell plays its chime first (A-05) and,
  once the chime has started, starts its radio on the other processor core and broadcasts one ring
  message on channel 1. The Indoor Ringer listens on channel 1 and rings once for each ring message it
  can read (contract `ring` v1.0, A-07). A power-on or other wake of the Doorbell doesn't ring the
  Ringer and doesn't start the radio. **On a version mismatch the Ringer rings anyway** (DL-038: for a
  doorbell, failing safe means ringing): a message with the ring contract's marker but another major
  version rings it and is reported on its serial port. Messages that aren't ours, are cut short, or
  are readable but not a ring are ignored. The Ringer still ignores a ring that
  arrives while it's chiming (INC-02, AC8).
- **Increment type:** behavior change (both units) and a new shared contract.
- **Acceptance criteria:**
  1. [Local] Encoding a v1.0 ring gives the golden sample `tests/local/golden/ring_1_0.bin` byte for
     byte. (AC1)
  2. [Local] Encode needs 8 bytes of room and writes nothing if it has less. (AC2)
  3. [Local] The golden sample decodes as a v1.0 ring with its ring number. (AC3)
  4. [Local] A newer minor version with extra bytes is still a ring. (AC4)
  5. [Local] A different major version fails safe: not a ring. (AC5)
  6. [Local] Other devices' messages aren't ours (including spike S-03's). (AC6)
  7. [Local] A cut-short message is rejected. (AC7)
  8. [Local] A readable message of another type isn't a ring. (AC8)
  9. [Local] A button wake sends one v1.0 ring carrying the ring number. (AC9)
  10. [Local] Each ring has the next number, wrapping after 65535. (AC10)
  11. [Local] Power-on and other wakes send nothing and use no number. (AC11)
  12. [Local] A failed send is reported, and the next ring still gets a new number. (AC12)
  13. [Gate] A-07: the contract declares `kMajor`/`kMinor` and has a golden sample. COMMON: each unit's
      copy of `firmware/common/logic/` matches the master. A-02, A-03, A-05 still pass. Both units build
      within B-01 and B-05.
  14. [Real] RT-04: 10 Doorbell presses give 10 Ringer chimes, numbered in order with none doubled; the
      Doorbell's power-on doesn't ring the Ringer; with the Ringer unplugged the Doorbell still chimes
      every time.
  15. [Real] RT-01 rerun with the radio working, from the same phone recording (DL-037): B-03
      press-to-sound ≤ 300 ms still holds.
- **Simulated tier:** none (Wokwi doesn't implement I2S on the ESP32-S3, and ESP-NOW between two
  simulated boards isn't supported).
- **Protected behavior:** all absolutes; baselines 1 and 2 (the INC-01 and INC-02 Local tests).
  The ring path (`logic/ring/`) is unchanged.
- **Risks:** broadcast and unencrypted: any ESP-NOW device in range can ring the Ringer (accepted,
  DL-037; rank 13). Broadcast has no radio-level retry: a lost packet is a missed ring (S-03: 20 of 20
  arrived). The Doorbell must not sleep before its send finishes: the hardware layer waits for it
  (up to 200 ms) before deep sleep. The Ringer's radio uses channel 1 until it joins Wi-Fi (rank 7,
  spike S-02). The ring delay end to end (S-03 measurement 2) isn't measured (DL-038). Version
  mismatch rings (DL-038): under a new major version the Ringer can't tell a ring from another
  message type.
- **Files in scope:**
  - `firmware/common/logic/contract/ring.h` (master; declarations and stub in the lock commit;
    generation replaces the two function bodies), copied by `tools/sync_common.py` into
    `firmware/doorbell/src/logic/common/` and `firmware/indoor-ringer/src/logic/common/`
  - `firmware/doorbell/src/logic/ports.h` (adds `RingLink`, lock commit)
  - `firmware/doorbell/src/logic/notify/notify.h` (lock commit) and `notify.cpp` (stub; generation
    replaces the body)
  - `firmware/doorbell/src/hw/` (ESP-NOW link on core 0 that waits for the chime to start; the audio
    wrapper signals the start; sleep waits for the send), `firmware/doorbell/doorbell.ino` (wiring)
  - `firmware/indoor-ringer/src/logic/listen/` (which decoded messages ring the Ringer, DL-038; added
    after the lock, so its test is a helper test in `tests/local/unlocked/`, not a locked check)
  - `firmware/indoor-ringer/src/hw/` (ESP-NOW receive: decodes with the contract and queues the
    arrival time for `takeRing()`), `firmware/indoor-ringer/indoor-ringer.ino` (wiring)
  - `tests/local/locked/test_inc03_ring_over_espnow.cpp`, `tests/local/golden/ring_1_0.bin`;
    `tests/real/RT-04-doorbell-rings-ringer.md`
- **Fits the mold?** Yes, after the wall change in this branch (COMMON and A-07 checks; DL-037).
- **Checks locked:** lock commit on branch `inc/03-ring-over-espnow`. Lock run: all 12 new Local
  checks compiled, ran and failed on their assertions against the stubs; none passed, so none was
  reclassified.
- **Real-tier procedures required:** RT-04 (with the RT-01 rerun); RT-02 and RT-03 regression by ear
  during RT-04.
