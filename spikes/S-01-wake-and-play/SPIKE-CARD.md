# Spike S-01 — Wake and Play (FIRM)

Status: **CLOSED 2026-11-09 by the Product Owner.** Run on Ron's bench 2026-11-09 (Ron ran it as approved work; closure recorded in chat). Spike code is throwaway: archived read-only at `claude/spikes/S-01-wake-and-play/` in the project, never merged.

## Spike card
- **Question:** On the ESP32-S3 SuperMini, can a button press wake the board from deep sleep, and can it then play a built-in sound through the I2S amplifier using the core's own `playWAV()` (Arduino ESP32 core 3.3.12)? How long does it take from button press to sound?
- **Why it matters:** decides the Doorbell's basic design (deep sleep + wake on button) and the audio approach (core library vs. third-party) before Decide. It also tests the Product Owner's expectation below.
- **Product Owner's expectation (2026-11-06):** waking and playing takes about **100–300 ms**, and **300 ms at worst is acceptable for a doorbell.** → If confirmed, this becomes a dynamic budget: **press-to-sound ≤ 300 ms**, with the warning line at 80% (240 ms).
- **Time box:** 2 hours on the bench.
- **What counts as an answer:**
  1. Wake on button: yes/no.
  2. `playWAV()` plays an audible sound through the I2S amp: yes/no.
  3. Press-to-sound time, from the phone-recording procedure (3+ presses), compared with 300 ms.
  4. The in-app timings from serial (app start → I2S ready / WAV built / play start).
  5. Notes on wiring, pins, or anything surprising.
- **Disposal:** `spikes/S-01-wake-and-play/` in the repo — the code, the serial logs and the recording — read-only, labeled, never merged.
- **Out of scope:** sleep current (no µA meter yet; Product Owner decision pending), the battery, the Indoor Ringer, ESP-NOW.

## Hardware (updated 2026-11-09 — DL-005)
Amplifier: **MAX98357A** (Ron has two; one per unit). The NS4168 board from the NULLLAB kit is a spare.
- The spike sketch sends the same sound on both stereo channels. With SD left unconnected, the MAX98357A typically mixes both channels, so this works for either amp.
- *Check that the board's pin labels read VIN, GND, SD, GAIN, DIN, BCLK, LRC before wiring. Clone boards sometimes differ.*

## Wiring (breadboard, USB power)
| SuperMini | MAX98357A | Notes |
|---|---|---|
| **5V** | **VIN** | Powered from USB for the spike |
| **GND** | **GND** | |
| **GPIO4** | **BCLK** | Bit clock |
| **GPIO5** | **LRC** | Word select |
| **GPIO6** | **DIN** | Data |
| — | **SD**, **GAIN** | Leave unconnected (defaults) |
| — | **+ / −** | 8-ohm speaker |
| Push button | **GPIO2** and **GND** | No resistor needed; the sketch enables the internal pull-up |

**Pins chosen to avoid:** 0, 3, 45, 46 (strapping), 19/20 (USB), 48 (onboard LED). All chosen pins are on the side headers. GPIO2 is an RTC pin, so it can wake the chip from deep sleep (ESP32-S3: RTC GPIOs are GPIO0–21). *Check that GPIO2/4/5/6 are labeled on your board's headers before wiring.*

**Change 2026-11-09:** the button moved from GPIO1 to **GPIO2** at Ron's request. GPIO2 is also an RTC pin and not a strapping pin, so the wake method is unchanged.

## Procedure
1. Arduino IDE settings: ESP32S3 Dev Module, USB CDC On Boot **Enabled**, Flash 4MB, PSRAM QSPI.
2. Upload `S-01-wake-and-play.ino`, press **RST**. Expect a ding-dong immediately (power-on), then the Serial Monitor shows the timings and "Going to deep sleep."
3. The USB port disappears while the board sleeps. **That's expected.**
4. Press the button: expect the ding-dong. Reopen/reselect the Serial Monitor port within 3 seconds to see the timings. If it reconnects too slowly, the sound itself is still the evidence.
5. **True press-to-sound time (Real tier):** record with a phone voice memo, mic close to the button and speaker. Press the button 3+ times, a few seconds apart. In any audio editor, measure from the button's click to the start of the tone for each press. Record each value.
6. Record the answers in the "Results" section below.

**Known limitation:** the serial timings start when the app starts, so they **exclude** the chip's ROM/bootloader time after waking. The phone recording measures the whole thing.

## Results — run 1 (2026-11-09, Ron's bench, MAX98357A, button on GPIO2, SAMPLE_RATE 24000)
Serial output (wake #2):
```
=== S-01 wake and play ===
Wake #2, cause: BUTTON (ext0)
I2S begin: OK | WAV built: OK (67244 bytes)
App start -> I2S ready:  0 ms
App start -> WAV built:  237 ms
App start -> play start: 237 ms
Play duration:           643 ms
Going to deep sleep. Press the button to wake. (USB will disconnect.)
```
- **Wake on button: YES.** The wake cause reads BUTTON (ext0), and the sound played.
- **`playWAV()` audible through the MAX98357A: YES** (on power-on and on button wake).
- I2S setup takes ~0 ms.
- **237 ms is spent building the sound** (computing 22,400 sine samples at runtime after each wake). This is a spike artifact, not a property of the design: the product's built-in sound will be a ready-made array stored in flash, which needs no computing.
- "Play duration" 643 ms is shorter than the 700 ms sound because `playWAV()` returns once the last data is handed to the I2S hardware; the tail is still playing. Noted, not a problem.
- **Still open:** true press-to-sound (phone recording). The serial times exclude the chip's ROM/bootloader start-up after wake. Total ≈ boot time + 237 ms.

## Results — run 2 (2026-11-09, power-on, sketch settings changed by Ron)
```
Wake #1, cause: power-on/reset
I2S begin: OK | WAV built: OK (89644 bytes)
App start -> I2S ready:  0 ms
App start -> WAV built:  316 ms
App start -> play start: 316 ms
Play duration:           666 ms
```
- **Sample rate: run 1 at 24 kHz, run 2 at 32 kHz** (confirmed by Ron: changed `SAMPLE_RATE` from 24000 to 32000 for run 2). Tone length unchanged at 0.35 s per tone. That explains the sizes: 67,244 bytes at 24 kHz and 89,644 at 32 kHz (the unmodified 16 kHz sketch makes 44,844).
- Build time grew in proportion to the sound's size (237 → 316 ms, ≈14 µs per sample frame). This confirms that computing the sound at runtime doesn't scale: **the built-in sound must be stored ready-made in flash.**

## Product Owner observation (2026-11-09)
- Ron, listening at the bench: "from a performance perspective this is good." The press-to-sound delay is acceptable by ear, even with the 237–316 ms spent computing the sound at runtime (which the product won't do).
- This is a human observation, not a measurement. The ≤ 300 ms figure stays a *proposed* budget until a Real-tier procedure measures it.

## Closure (2026-11-09)
| Question | Answer | Evidence |
|---|---|---|
| Wake from deep sleep on the button? | **Yes** (GPIO2, ext0 wake) | Run 1 serial: "cause: BUTTON (ext0)"; sound played |
| `playWAV()` through the MAX98357A? | **Yes**, core 3.3.12, no third-party library | Runs 1 and 2: sound on power-on and on wake |
| Press-to-sound ≤ 300 ms? | **Acceptable by ear (Product Owner)**; not measured | PO observation. ≤ 300 ms stays a *proposed* budget, measured by a Real-tier procedure in Construction |

**Findings carried into Decide**
- Computing the sound after each wake costs ~14 µs per sample frame (237 ms at 24 kHz, 316 ms at 32 kHz), so the built-in sound must be stored ready-made in flash.
- Sample rate drives file size (32 kHz is 2× the size of 16 kHz for the same sound). The rate is a Decide item, informed by the relay and storage budgets. **Answered 2026-11-09: 24 kHz sounds okay (DL-010).**
- Board behavior: the USB port disappears during deep sleep (expected); uploading to a sleeping board needs BOOT + RST.

**Not done (deliberately):** S-01b (sound stored in flash), the phone press-to-sound measurement, and sleep-current measurement (no µA meter). Volume settings weren't recorded.

**Decision log:** DL-006, DL-007, DL-008.
