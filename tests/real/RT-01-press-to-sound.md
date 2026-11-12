# Real-tier procedure RT-01: press-to-sound time (budget B-03)

- **Behavior being proven:** a button press is heard as the start of the built-in sound within
  300 ms (B-03; warning line 240 ms). Increment card: INC-01, criterion 6.
- **Setup:** Doorbell on the bench, wired as on the S-01 wiring sheet (button GPIO2 to GND;
  MAX98357A on GPIO4/5/6; 8 Ω speaker). Firmware: the INC-01 build under test (write down the commit).
  Powered from USB. A phone with a recorder app (record as WAV or MP3), and Audacity on a computer.
  Python 3 for the measurement script. Quiet room.
- **Steps:**
  1. Power the Doorbell and let the power-on chime finish, then wait 5 seconds.
  2. Start a voice memo with the phone's microphone about 10 cm from both the button and the speaker.
  3. Press the button firmly and release it at once. Wait for the chime to finish, then 3 more seconds.
  4. Repeat step 3 until there are 10 presses. Stop the recording.
  5. Open the recording in Audacity and export it as WAV (File > Export Audio > WAV).
  6. Measure it with the pinned script (DL-026):
     `python3 tools/measure_press_to_sound.py recording.wav`
     It lists each press with its click-to-chime time, the max and median, and a B-03 verdict.
  7. Cross-check one press by hand in Audacity: select from the click spike to where the chime's
     waveform starts. It should agree with the script within a few ms.
  8. If the script flags any press as AMBIGUOUS (more than one click before the chime), look at it in
     Audacity and decide what it was. If it was a real press, its time counts toward the pass rule.
- **Expected observation:** 10 clicks, each followed by one chime.
- **Pass rule:** at least 10 clean presses, and every ambiguous press resolved by a person. PASS if
  all counted times are ≤ 300 ms. If any time is over 240 ms, it's still a pass but
  the warning line is crossed: record it in the run log for the Verifier. FAIL if any time is over
  300 ms or any press gave no chime.
- **Result record:** date · who · commit under test · recorder app and format · the script's output
  (every press, max, median, verdict) · the hand cross-check · how each ambiguous press was resolved ·
  pass/fail · warning line crossed (yes/no) → `docs/firm/run-log.md`.
- **Method history:** until 2026-11-12 the times were measured by hand in an audio editor. From
  2026-11-12 the script is the method and the hand reading is the cross-check (DL-026).
