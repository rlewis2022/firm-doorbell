# Spike S-03 — Radio alongside the chime, and checking in (FIRM)

Status: **RECORDED 2026-11-11, not started.** Raised as an escalation during INC-01 (DL-024). Runs before
backlog rank 3. Spike code is throwaway: archived read-only under `spikes/S-03-radio-and-checkin/`,
never merged.

## Why this spike exists
In deep sleep the Doorbell's radio is completely off, so it can't hear the Indoor Ringer. Every press
already needs the radio for the ring message (rank 3), and later for new sounds (rank 8). Two things
we haven't measured:
- **Radio start-up time.** S-01 measured no radio. ESP-NOW doesn't join a network (A-03), but the
  radio still has to start, and that time isn't known.
- **The blocking chime.** `playWAV()` returns only when the 2.9 s chime ends. Starting the radio after it
  would make the Indoor Ringer ring about 3 s late. Starting it before it would break A-05 and B-03.

## Spike card
- **Question:** Can the Doorbell start its radio and send a message to the Indoor Ringer while the
  chime is playing, without delaying or disturbing the chime, and what does a check-in for updates
  cost in time and current?
- **Why it matters:** decides how rank 3 (ring message over ESP-NOW) is built, and how updates reach a
  Doorbell that's asleep (rank 4, the test-sound relay, after the DL-029 re-ranking; then rank 8): check in after each press, on a timer, or both, and how often.
  Product Owner's caution (2026-11-11): a check-in must never sit in the start-up path ahead of the sound.
- **Time box:** 3 hours on the bench (confirmed by Ron, 2026-11-12).
- **What counts as an answer:**
  1. Radio start-up time on the SuperMini: app start → ESP-NOW ready, in ms (serial timing, 5+ wakes).
  2. Ring message delay: Doorbell press → Indoor Ringer reaction, in ms (phone recording of both
     units, 5+ presses).
  3. Chime with the radio starting in parallel (audio on one core, radio on the other, or non-blocking
     audio): any audible glitch? yes/no, from listening and from the recording.
  4. Press-to-sound with the radio work added: still ≤ 300 ms (B-03)? Same method as RT-01.
  5. One check-in exchange ("anything new?" → "nothing new"): extra awake time in ms.
  6. Current while the radio is on, in mA (multimeter in series or a USB power meter). With 5, this
     gives the battery cost of a timer check-in at different intervals.
  7. Notes on anything surprising.
- **Disposal:** `spikes/S-03-radio-and-checkin/` in the repo — sketches for both units, serial logs,
  recordings, measurements — read-only, labeled, never merged.
- **Out of scope:** Wi-Fi on the Indoor Ringer and channel sharing (spike S-02); sending sound files;
  the contract design (rank 3 decides it, versioned per A-07); deep-sleep current (B-04, rank 5 after the DL-029 re-ranking).
- **Hardware:** both SuperMinis, both MAX98357A boards and speakers. The second unit runs a throwaway
  receiver sketch.

## Results
Not run yet.
