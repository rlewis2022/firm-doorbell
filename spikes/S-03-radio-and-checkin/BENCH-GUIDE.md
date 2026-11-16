# Spike S-03 — bench guide (time box: 3 hours)

Throwaway code. Card: `docs/firm/04-spike-S03-radio-and-checkin.md`. When the spike closes, these
sketches, your serial logs and recordings are archived read-only in `spikes/S-03-radio-and-checkin/`.

Both sketches were written without compiling them (the ESP32 core can't be downloaded in Claude's
workspace). If either one doesn't compile, send the first `error:` lines.

## What this spike tests
The Doorbell **sends** messages over ESP-NOW and the Indoor Ringer **receives** them. So for most steps
**both units must be powered and running at the same time**, side by side on the bench.

| Step | Doorbell | Indoor Ringer |
|---|---|---|
| 2. MODE 0 (sound only) | running | running (should stay silent) |
| **3. MODE 1 (the main test)** | running | **running** |
| 4. MODE 2 (radio after the chime) | running | **running** |
| 5. MODE 3 (current) | running, through the meter | not needed |

## Setup (once)
1. **Place both units side by side**, about 1 m (3 ft) apart. ESP-NOW reaches much further; distance
   doesn't matter here.
2. **Power:**
   - **Indoor Ringer:** its USB wall adapter (or a laptop USB port).
   - **Doorbell:** the laptop's USB port, because you'll upload to it several times.
3. **IDE settings, both boards:** ESP32S3 Dev Module, core 3.3.12, USB CDC On Boot **Enabled**,
   Flash 4MB, PSRAM QSPI, Partition **No OTA**.
4. **Two IDE windows**, one per board (each window shows one Serial Monitor):
   - Window 1: `S-03-ringer/S-03-ringer.ino`, Tools > Port = the Ringer's port.
   - Window 2: `S-03-doorbell/S-03-doorbell.ino` (keep `builtin_dingdong.h` beside it), Port = the
     Doorbell's port.
   If the Ringer is on the wall adapter you won't see its Serial Monitor; that's fine, its beep is the
   evidence.
5. These sketches replace INC-01 and INC-02 on the boards. Re-upload those afterwards (baselines 1 and 2).

## 1. Ringer (once)
(v2 of the Ringer sketch prints a `[status]` line every 5 s with its channel and how many packets it has received, and a `[packet]` line for every packet that arrives, recognised or not.)

Upload `S-03-ringer.ino` to the Ringer. **It beeps once when it's ready.** If its Serial Monitor is open,
press RST to see `S-03 Ringer listening on channel 1. My MAC: ...`. After that it prints only when a
message arrives from the Doorbell, so an empty Serial Monitor is normal until then.
**Leave the Ringer powered for steps 2–4.**

## 2. Doorbell, MODE 0 (sound only, baseline)
In window 2 set `#define MODE 0` and upload to the Doorbell (hold its button to keep it awake, as before),
then unplug and replug it. Press the Doorbell's button 3 times, waiting for the chime + 3 s each time.
After each chime the Doorbell **stays awake for 20 seconds** and prints its timing table every 2 seconds (headed `=== S-03 Doorbell v3 ...`), so the Serial Monitor has time to reconnect; then it sleeps. **Wait for the table before pressing again.** If nothing appears at all, the S-03 sketch isn't on the board (INC-01 prints nothing): re-upload and check the Output panel ends with `Writing at ... (100 %)`.
Copy the tables. **Expected:** no radio lines, and the Ringer stays silent.

## 3. Doorbell, MODE 1 (radio in parallel: the main question)
Set `#define MODE 1`, upload, unplug and replug. Start a phone recording (same app as RT-01), phone about
10 cm from the **Doorbell's button**, with the Ringer still within earshot. Press the Doorbell's button
**10 times**, waiting for the chime + 3 s each time. **Expected:** the Ringer beeps shortly after each
press, while the Doorbell is still chiming. Listen: is the Doorbell's chime clean, or does it stutter or
click while its radio starts?
Copy the Doorbell's timing tables (and the Ringer's lines if you can see them). Send the recording.

## 4. Doorbell, MODE 2 (radio after the chime, for comparison)
Set `#define MODE 2`, upload, press 3 times. Copy the tables. **Expected:** the Ringer beeps about 3 s
late, after the Doorbell's chime ends.

## 5. Doorbell, MODE 3 (current; the Ringer isn't needed)
Put a USB power meter (or the multimeter on its mA range, in series with the 5 V line) between the
power source and the Doorbell. Set `#define MODE 3`, upload, press once. Read the meter in each phase the
Serial Monitor announces: A radio on and idle (15 s), B radio off but awake (15 s), C asleep.
Note: in C the amplifier is still powered (SD unconnected), so "asleep" isn't the battery figure.

## Send Claude
The Doorbell's timing tables (MODE 0, 1, 2), the Ringer's lines if you have them, the MODE 1 recording,
the three meter readings, and anything surprising.
