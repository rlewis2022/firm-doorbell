# Real-tier procedure RT-05: the Doorbell runs on its battery (with the RT-01 rerun)

- **Behavior being proven:** the Doorbell works from its battery alone, with the amp powered from the
  battery and switched on only while a sound plays; the power LED is gone; USB charges the battery.
  B-03 (≤ 300 ms) still holds, measured with RT-01's script (DL-026). B-04 is **not** measured
  (DL-039). Increment card: INC-04, criteria 2–6.
- **Setup:**
  - Doorbell rewired (DL-039): the 2000 mAh LiPo on the B+/B− pads through a JST-PH 2.0 lead, **red to
    B+** (check the battery's red wire matches the lead's red wire before plugging in); amp **VIN to
    B+** (not 5V); amp **SD to GPIO7**; amp GND, BCLK=GPIO4, LRC=GPIO5, DIN=GPIO6 and the button
    (GPIO2 to GND) as before; the board's red power LED (lit on battery alone, next to
    the charger) removed; the blue charge LED (lit only with USB) kept. Write down how VIN reaches B+
    (wire soldered to the pad, or to the lead).
  - Both units built from the INC-04 build under test (write down the commit), in the Arduino IDE with
    core 3.3.12, ESP32S3 Dev Module, No OTA, USB CDC On Boot enabled. Upload to the Doorbell over USB,
    then unplug its USB.
  - The Indoor Ringer on USB, 2–3 m away, with its Serial Monitor open at 115200 baud and cleared.
  - A phone with a recorder app; Python 3. Quiet room.
- **Steps:**
  1. With the Doorbell's USB unplugged (battery only), look at both sides of the board.
  2. Press the Doorbell's reset button (RST) to restart it from power-on. Listen to both units for 5
     seconds.
  3. Start a recording with the phone about 10 cm from the Doorbell's button and speaker.
  4. Press the Doorbell's button firmly and release it at once. Wait for both chimes to finish, then
     3 more seconds. Repeat until there are 10 presses. Stop the recording. Copy the Serial Monitor's
     text.
  5. Wait 10 seconds after the last chime, then hold an ear close to the Doorbell's speaker for 10
     seconds.
  6. Run the pinned script on the recording (exported as WAV):
     `python3 tools/measure_press_to_sound.py recording.wav`
     Resolve any AMBIGUOUS press by hand, as in RT-01 step 8.
  7. Plug USB into the Doorbell (a USB charger or the computer). Look at both sides of the board.
     Press the button once.
  8. Unplug USB. Press the button once.
- **Expected observation:**
  1. No LED is lit on the board (the red power LED is gone).
  2. The Doorbell plays its power-on chime once, clean. The Ringer stays silent and prints no ring.
  4. Each press: the Doorbell chimes, clean and sounding as before (no new click or pop at the start
     or end, not distorted), and the Ringer chimes once. The Serial Monitor shows 10 ring lines, ring
     #0 to #9, none missing or repeated, no "ignored" lines.
  5. Silence from the Doorbell's speaker: no hiss or hum.
  6. 10 clean presses; B-03 verdict PASS (or PASS with the warning line crossed: record it).
  7. The blue charge LED lights (if the battery isn't already full; if it doesn't light, record whether
     the battery was freshly charged). No red LED lights. The press chimes and rings the Ringer.
  8. The press chimes and rings the Ringer.
- **Pass rule:** PASS only if every observation matches. Anything else is a FAIL; describe what
  happened. A silent Doorbell with a ringing Ringer at step 2 or 4 points at the amp's power or the
  SD_MODE channel selection (INC-04 risks).
- **Result record:** date · who · commit under test · how VIN reaches B+ · each step's observation ·
  the script's output · pass/fail → `docs/firm/run-log.md`.
