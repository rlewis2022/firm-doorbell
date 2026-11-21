# Increment card INC-04: the Doorbell runs on its battery

- **Backlog item:** rank 4 (Decide §5, re-ranked by DL-039): the Doorbell's battery power path, for
  release 1.0. Decisions: DL-035 (no firmware LED; remove the power LED), DL-039 (battery, amp on the
  battery, amp SD_MODE on GPIO7, B-04 not measured for release 1.0).
- **Requirement:** the Doorbell runs from its 3.7 V 2000 mAh LiPo on the board's B+/B− pads, with no
  USB, and does everything it did at Baseline 3: chime first on a button press, ring the Indoor Ringer,
  go back to deep sleep. The amp is powered from the battery (B+). The Doorbell switches the amp **on**
  (GPIO7 high) only for the time a sound plays and **off** (GPIO7 low) otherwise, including through
  deep sleep, so the amp's 2.4 mA idle draw (data sheet) isn't spent while the Doorbell sleeps. The
  board's power LED is removed. Plugging in USB charges the battery through the board's charger.
- **Increment type:** hardware change (wiring, LED removal) and a small hardware-layer firmware change
  (Doorbell only). No decision logic changes, so no new Local checks; the ring path is untouched.
- **Acceptance criteria:**
  1. [Gate] All absolutes pass; both units build within B-01 and B-05; the 30 existing Local tests
     (baselines 1–3) still pass.
  2. [Real] RT-05: the Doorbell plays its power-on chime when the upload restarts it (USB plugged in;
     revised after the lock, see the run log), and the Ringer stays silent.
  3. [Real] RT-05: on battery alone, 10 presses give 10 clean Doorbell chimes, sounding as before
     (no new click or pop, not distorted), and 10 Ringer chimes numbered in order.
  4. [Real] RT-05: B-03 press-to-sound ≤ 300 ms still holds on battery (RT-01 script, DL-026), now
     that the amp is switched on for each sound.
  5. [Real] RT-05: between presses the Doorbell's speaker is silent (no hiss or hum with an ear close
     to it), and no LED is lit on battery (the red power LED is removed).
  6. [Real] RT-05: with USB plugged in, the blue charge LED lights (battery not full), and the Doorbell
     still works; unplugged again, it still works.
- **Not in this increment:** measuring B-04 (no µA meter, DL-039; a known gap in release 1.0); a
  battery-level warning (rank 12, DL-035); checking in on a timer (DL-036).
- **Simulated tier:** none (Wokwi doesn't implement I2S on the ESP32-S3; power can't be simulated).
- **Protected behavior:** all absolutes; baselines 1–3 (the INC-01, INC-02 and INC-03 Local tests);
  RT-01 to RT-04 behavior, now on battery. The Indoor Ringer is unchanged.
- **Risks:** SD_MODE above 1.4 V selects the amp's left channel: the mono chime must come out on it
  (RT-05 step 4). Switching the amp on adds a little time before the sound and may click (RT-05 steps
  4 and 6). The breakout's own pull-up on SD_MODE draws some current while GPIO7 holds it low (size
  unknown without a meter). The 5V pin is dead on battery, which is why VIN moves to B+. The charger's
  type is taken from the board, not confirmed (step 8). LiPo polarity on JST-PH leads varies: check red
  to B+ before connecting.
- **Files in scope:** `firmware/doorbell/src/hw/i2s_audio.*` (amp on before each sound, off after),
  `firmware/doorbell/src/hw/esp32_power.*` (amp held off through deep sleep),
  `firmware/doorbell/doorbell.ino` (wiring: GPIO7); `tests/real/RT-05-doorbell-on-battery.md`.
- **Fits the mold?** Yes, after the wall change in this branch (DL-039: hardware facts in `CLAUDE.md`).
- **Before the lock (2026-11-20):** RT-05 steps 1, 2 and 7 revised from Ron's bench facts: the battery
  is soldered to the pads (it can't be unplugged, so step 2 restarts with the reset button); the red LED
  lit on battery alone is the power LED (to remove); the blue LED lit only with USB is the charge LED
  (kept). The amp's crackle on battery dropped once VIN moved from 5V to B+.
- **Checks locked:** lock commit on branch `inc/04-battery-path`. No new Local or Simulated checks
  (hardware-layer change only); RT-05 is written and locked before generation (Real-tier procedures
  are exempt from the fail-first lock run).
- **Real-tier procedures required:** RT-05 (includes the RT-01 rerun on battery).
