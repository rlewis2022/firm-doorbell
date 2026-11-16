# Spike S-03 — Radio alongside the chime, and checking in (FIRM)

Status: **CLOSED 2026-11-15** (Product Owner). Ran on the bench 2026-11-14/15. Decisions: DL-036. Spike code archived
read-only in `spikes/S-03-radio-and-checkin/`. Raised as an escalation during INC-01 (DL-024).

## Why this spike exists
In deep sleep the Doorbell's radio is completely off, so it can't hear the Indoor Ringer. Every press
already needs the radio for the ring message (rank 3), and later for new sounds (ranks 4 and 9 after re-ranking). Two things
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
  Doorbell that's asleep (rank 4, the test-sound relay, after the DL-029 re-ranking; then rank 9): check in after each press, on a timer, or both, and how often.
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

## Results (bench, 2026-11-14/15; Ron)

Setup: both units side by side, each on a USB data cable to the laptop. Doorbell: `S-03-doorbell.ino`
(v2, then v3); Ringer: `S-03-ringer.ino` (v1, then v2). Core 3.3.12, ESP32S3 Dev Module, No OTA. ESP-NOW
broadcast on channel 1 (both boards confirmed channel 1). Archived versions: Doorbell v3, Ringer v2.

| # | Measurement | Result |
|---|---|---|
| 1 | Radio start-up (app start → ESP-NOW ready) | `WiFi.mode(STA)` + channel **28.5–33.8 ms**; ESP-NOW init + peer **0.6–0.9 ms** (4 presses with full tables) |
| 2 | Ring message delay (press → Ringer) | **Not recorded** (no phone recording; Product Owner chose to skip). Serial: the ring is sent **29.9–35.2 ms after app start** in MODE 1. End-to-end delay is measured in rank 3 |
| 3 | Chime glitch with the radio starting in parallel | **None.** MODE 1, 10 presses: the Doorbell's chime was clean every time (Product Owner, by ear) |
| 4 | Press-to-sound with radio work added | **Not recorded** (no recording). Serial: audio starts **0.0 ms** after app start in every mode; the radio starts at the same moment on the other core. Rank 3's RT-01 run confirms it against B-03 |
| 5 | Check-in exchange ("anything new?" → "nothing new") | **2.7–3.2 ms** round trip |
| 6 | Current while the radio is on | **Not measured** (no USB power meter). Deferred to rank 5 |
| — | Reliability, MODE 1 | 10 presses: **20 of 20 packets** received by the Ringer (RING + CHECK? each), **10 of 10 beeps**, 10 of 10 check-in replies |
| — | Radio after the chime (MODE 2) | Radio starts at 2900.6 ms; ring sent **2930.3 ms** after app start: the Ringer would ring ~3 s late. Radio start-up itself unchanged (28.9 ms) |
| — | Boot → app start | 8.0–8.6 ms on earlier presses, ~25 ms on later ones (24.9, 25.2 ms); cause not determined |

Answer to the question: **yes.** The Doorbell can start its radio on the second core and send the ring
while the chime plays, with no audible effect on the chime, about 30–35 ms after the app starts. A
check-in costs ~3 ms of awake time when done during the chime. Its current cost (for timer check-ins)
is still unknown.

Bench notes:
- One early MODE 1 press gave no Ringer beep while the check-in reply came back; the Ringer had just
  been rebooted. Not seen again in 10 + 3 presses with the Ringer fully started.
- Several hours went to set-up problems, not the radio: a power-only USB cable; USB CDC On Boot
  disabled on the Ringer (no serial output); a Ringer upload that dropped at 36% (fixed by re-uploading);
  and the Serial Monitor not reconnecting after deep sleep (fixed in Doorbell v2: it stays awake 20 s
  after the chime and repeats its table). The original bench guide also didn't say both units must run
  together; it was rewritten during the spike.
- Spike spend: bench time wasn't recorded; it ran over two evenings, beyond the 3-hour time box, mostly
  on the set-up problems above. AI tokens: not available in this session.
- Ringer v1 printed its MAC as 00:00:00:00:00:00 (read too early); v2 reads it correctly.
