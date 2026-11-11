# Real-tier procedure RT-01: press-to-sound time (budget B-03)

- **Behavior being proven:** a button press is heard as the start of the built-in sound within
  300 ms (B-03; warning line 240 ms). Increment card: INC-01, criterion 6.
- **Setup:** Doorbell on the bench, wired as on the S-01 wiring sheet (button GPIO2 to GND;
  MAX98357A on GPIO4/5/6; 8 Ω speaker). Firmware: the INC-01 build under test (write down the commit).
  Powered from USB. A phone with a voice-memo app, and an audio editor that shows a waveform
  (e.g., Audacity). Quiet room.
- **Steps:**
  1. Power the Doorbell and let the power-on chime finish, then wait 5 seconds.
  2. Start a voice memo with the phone's microphone about 10 cm from both the button and the speaker.
  3. Press the button firmly and release it at once. Wait for the chime to finish, then 3 more seconds.
  4. Repeat step 3 until there are 10 presses. Stop the recording.
  5. Open the recording in the audio editor. For each press, mark the button's click (the first sharp
     spike) and the start of the chime (the first rise of the tone). Write down the time between them in ms.
- **Expected observation:** 10 clicks, each followed by one chime.
- **Pass rule:** PASS if all 10 times are ≤ 300 ms. If any time is over 240 ms, it's still a pass but
  the warning line is crossed: record it in the run log for the Verifier. FAIL if any time is over
  300 ms or any press gave no chime.
- **Result record:** date · who · commit under test · phone and editor used · the 10 times · max and
  median · pass/fail · warning line crossed (yes/no) → `docs/firm/run-log.md`.
