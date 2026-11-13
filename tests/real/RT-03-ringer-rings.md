# Real-tier procedure RT-03: the Indoor Ringer rings

- **Behavior being proven:** the Indoor Ringer chimes at power-on and on each press of its test button,
  gives one chime for a long hold, ignores presses while it's chiming, and is louder than the Doorbell.
  Increment card: INC-02, criteria 10 and 11. (Ring messages arrive with rank 3; they're proven at the
  Local tier here.)
- **Setup:** Indoor Ringer on the bench, wired with the same pins as the Doorbell (MAX98357A BCLK=GPIO4,
  LRC=GPIO5, DIN=GPIO6, VIN=5V, GND; test button GPIO2 to GND) and **GAIN through a 100 kΩ resistor to
  GND**. Firmware: the INC-02 build under test (write down the commit). Powered by USB. The Doorbell
  (baseline 1 or later) beside it, also on USB. A watch or phone timer.
- **Steps:**
  1. Plug in the Ringer's USB cable.
  2. Wait 10 seconds without touching anything.
  3. Press and release the test button. Wait for the chime to finish, then 3 more seconds.
  4. Repeat step 3 until there are 10 presses in all.
  5. Press and **hold** the test button for 6 seconds, then release. Wait 5 seconds.
  6. Press and release the test button, then press and release it again about 1 second later (while the
     first chime is still playing). Wait 5 seconds.
  7. Put the two units side by side, about 1 m from you. Press the Doorbell's button, wait for its chime,
     then press the Ringer's test button.
- **Expected observation:**
  1. One complete ding-dong (the power-on chime).
  2. Silence.
  3–4. One complete chime per press; 10 in all, none missed, cut short or doubled.
  5. Exactly one chime; no second chime while held or after release.
  6. Exactly one chime; the second press, made during the chime, is ignored.
  7. The Ringer is clearly louder than the Doorbell (Product Owner's judgment by ear).
- **Pass rule:** PASS only if every observation matches. Anything else is a FAIL; describe what happened.
- **Result record:** date · who · commit under test · each step's observation · pass/fail →
  `docs/firm/run-log.md`.
