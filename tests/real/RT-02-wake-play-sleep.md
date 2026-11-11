# Real-tier procedure RT-02: wake, play, sleep

- **Behavior being proven:** the Doorbell chimes on power-on and on every press, plays the whole
  sound once per press however long the button is held, and goes back to sleep. Increment card:
  INC-01, criterion 7.
- **Setup:** Doorbell on the bench, wired as on the S-01 wiring sheet. Firmware: the INC-01 build under
  test (write down the commit). Powered from a computer's USB port, with the Arduino IDE open on the
  Tools > Port menu, so you can see the port appear and disappear. A watch or phone timer.
- **Steps:**
  1. Plug in the USB cable.
  2. Wait 10 seconds without touching anything.
  3. Press and release the button. Wait for the chime to finish, then 5 more seconds.
  4. Repeat step 3 until there are 10 presses in all.
  5. Press and **hold** the button for 5 seconds, then release. Wait 10 seconds.
  6. Press and release the button once more.
- **Expected observation:**
  1. One complete ding-dong (the power-on chime), then the port disappears from the menu (asleep).
  2. Silence; the port stays gone.
  3. One complete ding-dong for each press; the port goes away again after each one.
  4. 10 complete chimes in all, none cut short, none missed, none doubled.
  5. Exactly one chime, no second chime while held or after release, and the port goes away after release.
  6. One complete chime (the Doorbell still works after a long press).
- **Pass rule:** PASS only if every observation above matches exactly. Anything else is a FAIL;
  describe what happened.
- **Result record:** date · who · commit under test · each step's observation · pass/fail →
  `docs/firm/run-log.md`.
