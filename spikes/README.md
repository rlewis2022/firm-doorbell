# Spikes (archive, read-only)

Throwaway experiments that answer one question each (FIRM). Their code **never** enters the product:
nothing here is built by the gate or included by firmware. A result worth keeping is rebuilt through
an increment card, inside the mold. Keep everything; never edit after closing. Corrections go in an `ERRATA.md` beside the spike.

| Spike | Question | Status | Decisions |
|---|---|---|---|
| S-01-wake-and-play | Button wake + `playWAV()` on the SuperMini/MAX98357A; press-to-sound time | Closed 2026-11-09. Errata 2026-11-11: see `ERRATA.md` (the sketch's comments name the wrong amplifier and sample rate) | DL-006, DL-007, DL-008, DL-010 |
| S-03-radio-and-checkin | Radio and ESP-NOW alongside the chime; cost of a check-in | Closed 2026-11-15 (Doorbell v3 and Ringer v2 archived; `BENCH-GUIDE.md` is the guide as last revised) | DL-036 |
