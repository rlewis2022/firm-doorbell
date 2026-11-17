# Real-tier procedure RT-04: the Doorbell rings the Indoor Ringer (with the RT-01 rerun)

- **Behavior being proven:** each Doorbell press rings the Indoor Ringer once over ESP-NOW; the
  Doorbell's power-on doesn't; the Doorbell still chimes when the Ringer is off (A-05); and the radio
  doesn't slow the Doorbell's chime (B-03 ≤ 300 ms, measured with RT-01's script, DL-026).
  Increment card: INC-03, criteria 14 and 15.
- **Setup:** both units built from the INC-03 build under test (write down the commit), in the Arduino
  IDE with core 3.3.12, ESP32S3 Dev Module, No OTA, USB CDC On Boot enabled. Doorbell wired as for
  RT-01; Ringer wired as for RT-03 (GAIN 100 kΩ to GND). Each on its own USB power. The Ringer about
  2–3 m from the Doorbell (same room). The Ringer's USB cable to the computer with the Serial Monitor
  open at 115200 baud. A phone with a recorder app; Audacity; Python 3. Quiet room.
- **Steps:**
  1. Plug in the Ringer. Wait for its power-on chime. The Serial Monitor shows that it's listening on
     channel 1.
  2. Plug in the Doorbell. Its power-on chime plays. Watch the Ringer for 5 seconds.
  3. Start a recording with the phone about 10 cm from the Doorbell's button and speaker.
  4. Press the Doorbell's button firmly and release it at once. Wait for both chimes to finish, then
     3 more seconds.
  5. Repeat step 4 until there are 10 presses. Stop the recording. Copy the Serial Monitor's text.
  6. Export the recording as WAV and run the pinned script (RT-01, DL-026):
     `python3 tools/measure_press_to_sound.py recording.wav`
     Resolve any AMBIGUOUS press by hand in Audacity, as in RT-01 step 8.
  7. Unplug the Ringer. Press the Doorbell's button 3 times, waiting for each chime to finish.
- **Expected observation:**
  1. One complete ding-dong from the Ringer; a "listening" line on channel 1.
  2. The Doorbell chimes once. The Ringer stays silent and prints no ring.
  4–5. For each press: the Doorbell chimes, and the Ringer chimes once. 10 Ringer chimes in all. The
     Serial Monitor shows 10 ring lines whose ring numbers go up by one each time, with none missing or
     repeated, and no "ignored" lines.
  6. 10 clean presses; B-03 verdict PASS (or PASS with the warning line crossed: record it).
  7. The Doorbell chimes all 3 times, sounding the same as before.
- **Pass rule:** PASS only if every observation matches. Anything else is a FAIL; describe what
  happened (for a missed Ringer chime, include the Serial Monitor text).
- **Note (not part of the pass rule):** the Ringer's chime overlaps the Doorbell's in the recording,
  so the script measures only the Doorbell's own press-to-sound. The end-to-end ring delay (spike
  S-03's measurement 2) would need a separate recording with the Doorbell's speaker disconnected;
  the Product Owner decides whether to make one.
- **Result record:** date · who · commit under test · each step's observation · the script's output ·
  pass/fail → `docs/firm/run-log.md`.
