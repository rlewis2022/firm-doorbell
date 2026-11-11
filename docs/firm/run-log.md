# Run log (FIRM)

One row per AI iteration. Recorded by the Engineer. Keep everything.

| # | Date | Increment | Model | Prompt / context (link or summary) | Commit | Gate result (which step failed) | Tokens in/out | Notes |
|---|---|---|---|---|---|---|---|---|

## Baselines

| Baseline | Date | Pull request | Merge commit | Gate on `main` | Integrator |
|---|---|---|---|---|---|
| 0 — the mold in place (no product behavior) | 2026-11-10 | #1 | 842b5aa | Passed ([run 42947604772](https://github.com/rlewis2022/firm-doorbell/actions/runs/42947604772)) | Ron |

## Lock runs

| Increment | Date | Lock commit | New Local checks | Result against the stub | Reclassified as protected behavior |
|---|---|---|---|---|---|
| INC-01 | 2026-11-11 | see `git log` on `inc/01-ring-from-flash` ("Lock INC-01 checks") | 7 (AC1–AC7, `tests/local/locked/test_inc01_ring.cpp`) | All 7 compiled, ran and failed on their assertions (doctest: 7 of 8 test cases failed; the baseline-0 test passed) | None |
