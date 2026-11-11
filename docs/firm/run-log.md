# Run log (FIRM)

One row per AI iteration. Recorded by the Engineer. Keep everything.

| # | Date | Increment | Model | Prompt / context (link or summary) | Commit | Gate result (which step failed) | Tokens in/out | Notes |
|---|---|---|---|---|---|---|---|---|
| 1 | 2026-11-11 | INC-01 | Claude Opus 5.5 (configured `claude-opus-5-5`) | Card INC-01 + `CLAUDE.md` + S-01 spike as reference (not copied). Wrote `logic/button.*`, `logic/ring/ring.cpp` (replaced the stub), `hw/i2s_audio.*`, `hw/esp32_power.*`, `doorbell.ino`; generated `src/sounds/builtin_dingdong.h` with `tools/wav2header.py` | see `git log` ("INC-01 generation") | Local: integrity, self-tests, absolutes and 8/8 Local tests passed; build and B-01 run in CI | Not available in this session | Open doubts: `playWAV()` returns nothing, so a bad WAV can't be detected (only I2S start failure returns false); mono slot mode with the MAX98357A not yet heard; button bounce on release could cause a second wake (RT-02 will show it); amp draws current while the chip sleeps (SD pin floating) — rank 4 |

## Baselines

| Baseline | Date | Pull request | Merge commit | Gate on `main` | Integrator |
|---|---|---|---|---|---|
| 0 — the mold in place (no product behavior) | 2026-11-10 | #1 | 842b5aa | Passed ([run 42947604772](https://github.com/rlewis2022/firm-doorbell/actions/runs/42947604772)) | Ron |

## Lock runs

| Increment | Date | Lock commit | New Local checks | Result against the stub | Reclassified as protected behavior |
|---|---|---|---|---|---|
| INC-01 | 2026-11-11 | see `git log` on `inc/01-ring-from-flash` ("Lock INC-01 checks") | 7 (AC1–AC7, `tests/local/locked/test_inc01_ring.cpp`) | All 7 compiled, ran and failed on their assertions (doctest: 7 of 8 test cases failed; the baseline-0 test passed) | None |
