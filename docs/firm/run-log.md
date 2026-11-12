# Run log (FIRM)

One row per AI iteration. Recorded by the Engineer. Keep everything.

| # | Date | Increment | Model | Prompt / context (link or summary) | Commit | Gate result (which step failed) | Tokens in/out | Notes |
|---|---|---|---|---|---|---|---|---|
| 1 | 2026-11-11 | INC-01 | Claude Opus 5.5 (configured `claude-opus-5-5`) | Card INC-01 + `CLAUDE.md` + S-01 spike as reference (not copied). Wrote `logic/button.*`, `logic/ring/ring.cpp` (replaced the stub), `hw/i2s_audio.*`, `hw/esp32_power.*`, `doorbell.ino`; generated `src/sounds/builtin_dingdong.h` with `tools/wav2header.py` | see `git log` ("INC-01 generation") | Local: integrity, self-tests, absolutes and 8/8 Local tests passed; build and B-01 run in CI | Not available in this session | Open doubts: `playWAV()` returns nothing, so a bad WAV can't be detected (only I2S start failure returns false); mono slot mode with the MAX98357A not yet heard; button bounce on release could cause a second wake (RT-02 will show it); amp draws current while the chip sleeps (SD pin floating) — rank 4 |

## Baselines

| Baseline | Date | Pull request | Merge commit | Gate on `main` | Integrator |
|---|---|---|---|---|---|
| 0 — the mold in place (no product behavior) | 2026-11-10 | #1 | 842b5aa | Passed ([run 42947604772](https://github.com/rlewis2022/firm-doorbell/actions/runs/42947604772)) | Ron |
| 1 — INC-01: the Doorbell rings from flash | 2026-11-12 | #5 (merged with #6) | 148b246 | Passed; Verifier review accepted 2026-11-11; RT-02 and RT-01 passed 2026-11-12 (RT-01: 16 clean presses, max 107.2 ms) | Ron |

## Lock runs

| Increment | Date | Lock commit | New Local checks | Result against the stub | Reclassified as protected behavior |
|---|---|---|---|---|---|
| INC-01 | 2026-11-11 | see `git log` on `inc/01-ring-from-flash` ("Lock INC-01 checks") | 7 (AC1–AC7, `tests/local/locked/test_inc01_ring.cpp`) | All 7 compiled, ran and failed on their assertions (doctest: 7 of 8 test cases failed; the baseline-0 test passed) | None |
| INC-02 | 2026-11-12 | see `git log` on `inc/02-ringer-rings` ("Lock INC-02 checks") | 9 (AC1–AC9, `tests/local/locked/test_inc02_ringer.cpp`) | All 9 compiled, ran and failed on their assertions (doctest: 9 of 17 test cases failed; the 8 existing tests passed) | None |

## Increment status

| Increment | Machine gate | Verifier code review | Real tier | Integrated | Baseline |
|---|---|---|---|---|---|
| INC-01 | Passed (branch run 43015981086; passed again on `main` after the merge) | **Accepted as written** by Ron, 2026-11-11, reviewing the code on GitHub | RT-02 and RT-01: **PASSED** 2026-11-12 (see Real-tier results) | 2026-11-11, **before the Real tier**: merging PR #6 also brought in INC-01's commits, because #6 was stacked on #5 (an Engineer error; pull requests won't be stacked again). Left in place by decision of the Product Owner. | **Baseline 1**, 2026-11-12 |

## Real-tier results

| Procedure | Date | Who | Firmware under test | Observations | Result |
|---|---|---|---|---|---|
| RT-02 wake, play, sleep | 2026-11-12 | Ron | INC-01 as merged on `main` (code unchanged since 2b02d31), built in Arduino IDE with core 3.3.12, ESP32S3 Dev Module, No OTA | 1: chimed once at power-on, then the port disappeared · 2: silent for 10 s · 3–4: 10 presses, 10 complete chimes, port gone after each · 5: 5+ s hold gave exactly one complete chime · 6: a normal press after the hold gave one complete chime | **PASS** |
| RT-01 press-to-sound (B-03 ≤ 300 ms, warning 240 ms) | 2026-11-12 | Ron (recording); Claude (measurement); Ron (Audacity reading) | Same as RT-02 | Recorded with the phone app "Voice Recorder" (by Nguyen Thi Kim), MP3, mono, 44.1 kHz, 128 kbps, ~10 cm from button and speaker. Measured with `tools/measure_press_to_sound.py` (DL-026): **16 clean presses**, click-to-chime 107.0, 107.2, 107.1, 107.1, 107.1, 107.1, 106.9, 107.2, 106.7, 107.2, 106.5, 107.0, 107.0, 105.3, 105.8, 105.8 ms (**max 107.2, median 107.0**; 36% of B-03); 12 other sounds excluded. **1 ambiguous event at 1:19**: a chime starts at 79.60 s with three spikes before it (79.16 s, 79.44 s, ~79.57 s) instead of one click, so it isn't clear what started it (a press would mean 440, 164 or ~30 ms). Correction: a first, numpy-based analysis reported 14 presses; it missed 2 real presses (1:24.9, 1:33.1) and didn't flag the 1:19 event. Ron's own reading in Audacity: "sub 100 ms" (overall impression) | **PASS**, approved by Ron (Verifier) 2026-11-12. The 1:19 event was resolved by the Verifier's approval; its cause wasn't identified. Watch for the same pattern at the next RT-01 |

Bench notes (2026-11-12): the first build failed with only "exit status 1"; the IDE had the generic `esp32_family` board selected (no partition scheme), and core 3.3.7 was installed. Fixed by selecting ESP32S3 Dev Module and installing core 3.3.12 (the pinned version). The board's BOOT and RST buttons can't be reached (it's mounted upside down on the breadboard), so INC-01 was uploaded by holding the doorbell button to keep the board awake, and reset by unplugging and replugging USB.
