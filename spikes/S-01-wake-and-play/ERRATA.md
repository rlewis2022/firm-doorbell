# S-01 errata (added 2026-11-11)

The spike was closed on 2026-11-09. Its files in this folder are kept exactly as they were and are
**not** edited. This note corrects them.

## 1. Amplifier: both runs used the MAX98357A
`S-01-wake-and-play.ino` says otherwise in its comments:
- **Line 3:** "…through the I2S amp (NS4168 board from the NULLLAB kit; MAX98357A also works)"
- **Lines 18–19:** "Stereo on purpose: the NS4168 plays only ONE channel (its CTRL pin selects which)…"

Those comments were written before the amplifier was confirmed and were never updated.
**Correct:** run 1 and run 2 both used the **MAX98357A** (DL-005; spike record, "Results"). The NS4168 board
was never used in the spike; it's a spare. The sketch sends the same sound on both stereo channels; with
SD unconnected the MAX98357A mixes both, so the stereo choice did no harm.

## 2. Sample rate: the runs used 24 kHz and 32 kHz, not 16 kHz
The sketch's comments (line 18) and `SAMPLE_RATE` (line 22) say 16 kHz. Ron changed `SAMPLE_RATE` on his
bench before running it:
- **Run 1:** 24,000 Hz (67,244-byte WAV)
- **Run 2:** 32,000 Hz (89,644-byte WAV)

The archived sketch is the version as written, at 16 kHz. To repeat run 1 or run 2, change `SAMPLE_RATE`.

Sources: decision log DL-005 and DL-010; `docs/firm/04-spike-S01-wake-and-play.md`, results for runs 1 and 2.
