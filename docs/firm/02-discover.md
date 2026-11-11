# ESP32 Doorbell — Elaboration: Discover Findings (FIRM)

Status: **IN PROGRESS.** Started 2026-11-05. Discover records evidence (what tools, documents and measurements show), not assumptions. Each finding states its source.

## D1. Board facts — measured on Ron's board (2026-11-05)
**Source:** a diagnostic sketch run on one of Ron's boards (silkscreen: HW-747 v0.02), Arduino IDE, board "ESP32S3 Dev Module", PSRAM "QSPI PSRAM". Output:

```
Model: ESP32-S3  Rev: 2  Cores: 2  CPU: 240 MHz
Flash: 4194304 bytes
PSRAM: 2097152 bytes
SDK: v5.5.2-729-g87912cd291
```

**Findings**
- Chip: ESP32-S3, 2 cores, running at **240 MHz**. The Amazon listing's "160MHz" is wrong.
- **Flash: 4 MB** (4,194,304 bytes). Firmware and sound files share this. → Sets the storage budget in Decide.
- **PSRAM: 2 MB** (2,097,152 bytes), present and working with "QSPI PSRAM" selected.
- This matches the commonly reported ESP32-S3FH4R2 configuration (4 MB flash, 2 MB PSRAM). The chip's printed marking was not read; the measured sizes are the evidence.
- The Espressif SDK (ESP-IDF) is v5.5.2.
- **Arduino ESP32 core: 3.3.12** (reported by Ron from Boards Manager, 2026-11-06). Arduino IDE: 2.3.8.
- Only one board was tested. The other boards in the pack are assumed to be the same until checked. → Check each board with the same sketch before it goes into a unit.

## D2. Working Arduino IDE settings for this board (verified 2026-11-05)
| Setting | Value |
|---|---|
| Board | **ESP32S3 Dev Module** (not "ESP32 Dev Module") |
| USB CDC On Boot | **Enabled** (otherwise Serial output doesn't reach USB) |
| Flash Size | 4MB (32Mb) |
| PSRAM | **QSPI PSRAM** (the default is Disabled; never use OPI) |
| CPU Frequency | 240MHz |

**Board quirks observed (workshop-worthy):**
- Out of the box, the board's USB port kept appearing and disappearing, consistent with a boot loop in the factory firmware.
- Recovery: hold **BOOT**, plug in, release. The board enters download mode and the port stays stable.
- After an upload, "Hard resetting via RTS pin" did **not** restart the board. Press **RST** manually; the COM port may change after the reset.
- The two buttons are BOOT and RST. Labels are tiny; pressing RST makes the port drop and return.

## D3. Existing code reviewed — not adopted (2026-11-05)
| Repo | What it is | Why not adopted |
|---|---|---|
| mtekman/doorbell-camera (GPL-3.0, last commit Jan 2025) | Camera doorbell: PlatformIO, camera web server, video recording, motion sensor, MQTT | About 97% is camera code. The remaining Wi-Fi code waits on the network, which breaks the candidate absolutes. No ESP-NOW or audio. GPL-3.0 would constrain the repo's license. |
| marrobHD/ESP32Doorbell (Apache-2.0, last commit Jun 2021) | ESPHome YAML + Home Assistant; camera, OLED, weather; ringtone plays on a Home Assistant media player | Not C++/Arduino. The ring depends on Wi-Fi and Home Assistant. No ESP-NOW or local audio. |
| petrepa/ESPHome-VideoDoorbell (MIT, last commit Aug 2023) | ESPHome YAML for a TTGO T-Camera; wired power; ring = Google text-to-speech on smart speakers via Home Assistant | Not C++/Arduino. The ring depends on Wi-Fi, Home Assistant and a cloud service. No battery, ESP-NOW or local audio. |

**Pattern:** every existing project reviewed is a camera doorbell built around Wi-Fi and a home hub. None does two units, ESP-NOW, local audio, and ringing that works with the network down. That gap is what this demo fills.

**Carried forward (as ideas only, no code copied):** wake from deep sleep on the button (to be written from Espressif's documented sleep API); an illuminated button (a possible later backlog item); a published 3D-printed enclosure as a design reference only.

## D4. ESP-NOW — from Espressif's documentation (2026-11-05)
**Sources:** ESP-IDF ESP-NOW API reference for ESP32-S3: the release-v5.4 page and the current stable page (v6.1). The board runs ESP-IDF v5.5.2, which falls between them; both pages agree on the points below. Also the Arduino core's ESP_NOW library source (arduino-esp32 master, version 3.3.12).

- **Packet size:** ESP-NOW v2.0 carries up to **1,470 bytes** per packet (`ESP_NOW_MAX_DATA_LEN_V2`); v1.0 carries 250. A v2.0 device receives both; a v1.0 device can't receive v2.0 packets longer than 250 bytes. Both units run the same core, so both are v2.0. The Arduino ESP_NOW library reports the limit at runtime via `getMaxDataLen()`.
- **Delivery is NOT guaranteed to the application.** The send callback's "success" only means the packet was received at the MAC (radio) layer. Espressif's own guidance: add application-level acknowledgments, retransmit on timeout, and use sequence numbers to drop duplicates. → **This must be in the file-relay contract.** It also directly supports Ron's fallback rule: the Doorbell must verify the whole file before using it.
- **Pacing:** don't send too fast. Send the next packet after the previous send callback has returned, or callbacks can arrive out of order. If `esp_now_send()` returns `ESP_ERR_ESPNOW_NO_MEM`, wait and retry.
- **Callback rule:** the callbacks run in a high-priority Wi-Fi task. Keep them short and hand data to a lower-priority task through a queue. → A candidate absolute (checkable by review and by a rule on callback length).
- **Channel:** a peer's channel must match the channel the device's station (or access point) is on; 0 means "the current channel." → Confirms the Inception risk: when the Indoor Ringer joins the router, ESP-NOW runs on the router's channel, and the Doorbell has to be on the same one. Still a spike.
- **Peers:** up to 20 paired peers; up to 17 encrypted (default 7). Two units is well within this. Encryption is available; whether to use it is a Decide question.
- **Power saving:** a wake window/interval exists, but sleep is only supported in station mode. For the Doorbell, deep sleep between presses (waking on the button) is the likely pattern, not ESP-NOW's power-save mode. To be confirmed by measurement.

## D5. Audio playback and storage — from the Arduino ESP32 core source (2026-11-05)
**Source:** arduino-esp32 repository, master branch (package version 3.3.12): `libraries/ESP_I2S`, `tools/partitions`, `boards.txt`. **Verified 2026-11-06: this is the same version Ron has installed (3.3.12).**

- **The core's own I2S library (ESP_I2S) can play audio held in memory:** `playWAV(data, len)` for short PCM WAV, and `playMP3(src, len)` for short MP3. The MP3 function only exists when the core is built with an MP3 decoder (`ARDUINO_HAS_MP3_DECODER`). → A built-in library may be enough, with no third-party dependency. To be confirmed in the spike.
- Pins are set with `setPins(bclk, ws, dout)`, which matches the MAX98357A's three signal pins.
- **Flash layouts available for a 4 MB ESP32-S3** (Arduino IDE → Tools → Partition Scheme):

| Scheme | App space | File system space | Over-the-air updates |
|---|---|---|---|
| Default 4MB | 1.25 MB ×2 | **1.375 MB** | Yes |
| No OTA (2MB APP/2MB SPIFFS) | 2 MB | **1.875 MB** | No |
| Huge APP (3MB No OTA/1MB SPIFFS) | 3 MB | 0.875 MB | No |

  The "SPIFFS" partition can also hold a LittleFS file system.
- **Size arithmetic (illustrative, not yet a decision):** an uncompressed 16-bit mono WAV at 16 kHz is 32 KB per second, so a 3-second chime is about 96 KB. At 1,470 bytes per ESP-NOW packet, that's roughly 66 packets before protocol overhead. At 250 bytes it would be about 390. → The sound-file size budget and the audio format are Decide items, informed by the spike.
- **The built-in sound** can be compiled into the firmware itself, so it never depends on the file system.

## D6. Hosting cost — DigitalOcean (2026-11-05)
**Source:** a third-party price summary (onedollarvps.com, "last reviewed July 6, 2026"). DigitalOcean's own pricing pages couldn't be opened from this session. → **Ron confirms the price on DigitalOcean's site when creating the Droplet.**

| Plan | Price/month | Memory | vCPU | Disk | Transfer |
|---|---|---|---|---|---|
| Basic, 512 MB | $4 | 512 MiB | 1 | 10 GiB | 500 GiB |
| Basic, 1 GB | $6 | 1 GiB | 1 | 25 GiB | 1,000 GiB |
| Basic, 2 GB | $12 | 2 GiB | 1 | 50 GiB | 2,000 GiB |

Weekly backups add 20% of the Droplet price. → The $4 or $6 plan (about $4.80–$7.20 with weekly backups) fits the $5–10/month ceiling; the $12 plan does not.

## D7. Open items in Discover
- [x] Arduino core version: **3.3.12** (D1).
- [x] **Battery pads are present** on the back of Ron's board (2026-11-06). Still open: whether there's a BOOST jumper, and the charge current (sources disagree: 100 mA fixed vs. 300 mA via BOOST). Confirm before connecting a battery.
- [ ] **Deep-sleep current of the board: unmeasured.** No published board measurement exists; the chip's datasheet figure (~7 µA) excludes the charger, LEDs and regulator. **Ron has no µA-capable meter (2026-11-06)**, so it's split out of spike S-01. The monthly-recharge goal depends on it; **Decided 2026-11-09 (DL-009): measure with a multimeter that has a µA range**, which Ron will obtain. Becomes a Real-tier procedure before the battery work.
- [x] ESP-NOW limits and behavior → D4. (Channel coexistence still needs the spike.)
- [x] Audio playback options → D5 (to be confirmed against Ron's installed core and in the spike).
- [x] Partition layouts within 4 MB → D5. File system choice → Decide.
- [x] DigitalOcean prices → D6 (Ron confirms on DigitalOcean's site).
- [ ] Wokwi: how the SuperMini is represented (likely as a generic ESP32-S3 board); confirm before writing the simulation scenarios.
