# ESP32 Doorbell — Project Inception (FIRM)

Status: **APPROVED — Purpose gate passed 2026-10-28; amended by the Product Owner 2026-11-05** (board and deadline; see Amendments). Drafted 2026-10-27. Product Owner: Ron.

Inception answers *why this work exists and what it has to live within*. Per FIRM, it contains no technology choices except the ones imposed from outside, and no backlog. Both come in Elaboration.

## Roles
Ron holds all seven roles (Product Owner, Architect, Context Engineer, Engineer, Verifier, Integrator, Operator), separated in time. The Verifier's hat goes on twice: to lock the checks before the first prompt, and to verify in a separate sitting afterward. The AI (Claude) is a tool the Engineer uses. It is never a role and never accountable.

## Purpose
A two-unit doorbell:

- a **battery-powered Doorbell** at the door, which also plays a sound outside so the visitor knows it worked
- a **USB-powered Indoor Ringer** inside the house

The two are linked by ESP-NOW. A simple web backend (login required) lets the user upload sound files and choose which one plays. When a sound is selected, the Indoor Ringer downloads it and relays it to the Doorbell, and both play it from local storage.

It serves two audiences:

1. **The household.** A visitor presses the button and the people inside hear it. This keeps working when Wi-Fi, the internet or the backend is down.
2. **FIRM readers and workshop attendees.** A small, real, open-source system that shows the method from end to end, with every increment, spike, run log and baseline kept.

## Long-term goals
- Ringing is reliable, including when the network or backend is unavailable.
- Sound selection and other controls (such as volume) are managed from the backend.
- The backend has a login. It serves a single user/family for now, but the design shouldn't rule out more users later.
- The project serves as the FIRM workshop/companion example: all artifacts are kept read-only and labeled, and the repo is public (read-only) from day one.
- **Out of scope for this project:** the camera with motion-triggered photos (noted earlier as a possible later phase); multiple users or households.

## Constraints imposed from outside

**Schedule**

- **Self-imposed deadline: 2026-11-17** (originally 2026-11-10; moved one week on 2026-11-05).

**Cost**

- Hosting: keep it low, **about $5–10 per month** total.

**Hardware**

- Board: **ESP32-S3 SuperMini on both units** (Ron has several). Commonly built on the ESP32-S3FH4R2 chip: **4 MB flash, 2 MB quad-SPI PSRAM**. Unbranded boards vary by factory, so the exact variant is confirmed from the chip marking in Discover. *(Originally the ESP32-S3-DevKitC-1; changed 2026-11-05.)*
- Audio on each unit: MAX98357A I2S amplifier and an 8-ohm speaker. The Indoor Ringer is designed to be louder.
- Doorbell: rechargeable battery, recharged roughly monthly, with a simple status/charging LED. Battery model not yet chosen.
- Indoor Ringer: USB wall-adapter power, plus a reset/config button. All controls (volume, etc.) go through the backend.
- Currently breadboard prototypes. Enclosures will be 3D printed later. Parts are bought on Amazon.

**Software and tools**

- Firmware: C++ with the Arduino framework, developed in the **Arduino IDE**. The pipeline builds with **arduino-cli** (same core and libraries).
- Backend: **Node.js**, edited in Notepad++, hosted on a **DigitalOcean Droplet**.
- Source and pipeline: a **public GitHub** repo with **GitHub Actions**. Because the repo is public from day one, **no secrets are ever committed**: no Wi-Fi credentials, backend passwords, keys or tokens. That's an absolute from the first commit, with secret scanning in the gate.
- Simulator: **Wokwi, free plan.** Verified limits: no ESP-NOW and no multiple boards in one simulation; no I2S on the ESP32-S3; no Bluetooth. The public gateway only reaches internet hosts and its traffic is monitored, so **test data only**. CI gets 50 simulation minutes per rolling 30 days, with a 5-minute limit per run.

**Behavior and communication**

- The units talk to each other over ESP-NOW. Only the Indoor Ringer talks to the backend, over Wi-Fi. **The Doorbell never uses Wi-Fi**, so it holds no Wi-Fi credentials.
- **Sound delivery:** the Indoor Ringer downloads the selected sound from the backend and relays it to the Doorbell over ESP-NOW.
- **Fallback rule (both units):** if the selected sound file doesn't arrive complete and intact, the unit plays its **built-in sound**. It never plays a partial or corrupted file, and it does **not** fall back to a previously downloaded sound. *Why:* an older custom sound playing after a new one was selected would leave the user unsure what happened. The built-in sound is an unmistakable signal that the new sound didn't arrive.
- Both units play audio from local storage, and both must work when the network fails.

**Method and book**

- Every iteration is recorded in the run log (model, prompt, context, commit, gate results, tokens).
- All artifacts are kept, read-only and labeled.
- Naming: "**Indoor Ringer**," never "chime."

## Critical integrations (named here; designed in Elaboration)
1. **Doorbell ↔ Indoor Ringer (ESP-NOW):** the ring event, configuration from the Indoor Ringer to the Doorbell, and **the sound file itself**, relayed in small packets. The Doorbell must be able to tell a complete, intact file from a partial or corrupted one. Versioned major.minor.
2. **Indoor Ringer ↔ Backend (Wi-Fi):** reading configuration and downloading the selected sound file. The Indoor Ringer must be able to tell a complete, intact download from a partial or corrupted one. Versioned major.minor.
3. **User's browser ↔ Backend:** logging in, uploading sounds, and selecting the active sound and settings.
4. **Sound file format on the devices:** an implicit contract between what the backend accepts and what the devices can store and play.
5. **Wokwi:** not a production integration, but it limits what can be checked at the Simulated tier.

## High-level success measures (refined in Elaboration)

**Product**

- The bell rings, inside and out, with the network and backend down (Real tier).
- Selecting a sound in the backend puts that sound on both devices, and they play it.
- A partial or corrupted file is never played. The built-in sound plays instead, on either unit.
- The Doorbell's battery lasts about a month between charges (becomes a dynamic budget).
- Delivered by 2026-11-17.

**Spend:** cost per accepted increment (tokens and time), as a trend; hosting at or under $10/month.

**Development time:** iterations to acceptance; regressions introduced; locked-check revisions (with reasons).

**Maintainability:** complexity trend; budget consumption; dependency count, with each dependency tied to a recorded decision.

**Method:** gate rejections (and which absolutes caught them); escaped defects.

## Known risks (carried into Elaboration)
- **Relaying sound files over ESP-NOW.** A file has to be split into many small packets and reassembled. Transfer time, reliability, the largest workable file size and the battery cost are all unmeasured. → Spike.
- **ESP-NOW and Wi-Fi sharing a channel on the Indoor Ringer.** When the Ringer joins the router, ESP-NOW runs on the router's channel. If the router changes channel, the link to the Doorbell can break. → Spike.
- **Battery life** with ESP-NOW, deep sleep and file transfers. The monthly recharge target hasn't been measured yet.
- **Device storage** for sound files on the ESP32-S3 (file size and format limits). → Becomes a budget in Decide.
- **Schedule.** Two weeks is tight for two firmwares, a backend with login, and the FIRM record-keeping. The backlog ranking in Elaboration has to protect the "rings with the network down" core first.
- **Simulator gaps (listed above)** push radio and audio checks to the Real tier.

## Candidate absolutes surfaced in Inception (for Decide)
- No secrets are ever committed (public repo).
- The Doorbell never uses Wi-Fi.
- Neither unit plays a sound file it hasn't verified as complete and intact. If verification fails, it plays the built-in sound.
- Ringing never waits on the network, the backend or a file transfer.

## Answers recorded 2026-10-27
1. Deadline: self-imposed, 2 weeks.
2. Sounds are uploaded to the backend through the web platform, and downloaded to the ESP when selected in the platform UI.
3. Yes, the Doorbell plays a sound outside too.
4. A login is required; single user/family for now.
5. The repo is public (read) from day one.
6. Hosting cost: about $5–10 per month.
7. Both units: ESP32-S3 (now the ESP32-S3 SuperMini; see Amendments).

- A. Doorbell route: relay over ESP-NOW; it defaults to a built-in sound if the audio file doesn't transmit fully or arrives corrupted.
- B. On a failed new file: the built-in sound, always — otherwise the user can get confused about what happened.
- C. The Indoor Ringer has the same fallback: yes.

## Amendments after the Purpose gate
- **2026-11-05 — Board:** both units use the **ESP32-S3 SuperMini** instead of the ESP32-S3-DevKitC-1. Consequences carried into Discover/Decide:
  - Storage is tighter (commonly 4 MB flash), so the sound-file budget gets smaller.
  - Some SuperMini versions have an onboard battery charger and battery pads, which could serve the Doorbell's battery. This needs checking on Ron's boards.
  - The onboard RGB LED is usually on GPIO48.
  - The boards use native USB ("USB CDC On Boot" must be enabled).
  - Pins to avoid: strapping pins 0, 3, 45, 46; USB pins 19/20.
- **2026-11-05 — Deadline:** moved one week, to **2026-11-17**.
- **2026-11-05 — Existing code:** three open-source doorbells were reviewed and **not adopted** (mtekman/doorbell-camera, marrobHD/ESP32Doorbell, petrepa/ESPHome-VideoDoorbell). All three are camera doorbells that rely on Wi-Fi and a home hub; none uses ESP-NOW or local audio. The only idea carried forward is waking from deep sleep on the button, which will be written fresh from Espressif's documented sleep API. No code is copied. Recorded in the Discover findings.
- **2026-11-05 — Repo:** not yet created. Ron will create it.

## Purpose gate (small system: checklist, signed by the Product Owner)
- [x] Purpose and audiences agreed
- [x] Long-term goals agreed, including what's out of scope
- [x] Constraints complete
- [x] Critical integrations named
- [x] High-level success measures agreed
- [x] Open questions answered or recorded as risks
- [x] Signed: Product Owner **Ron** — Date **2026-10-28** (approved in chat: "Approve.")

## Decisions recorded in this session (2026-10-27)
- Toolchain: Arduino IDE for development; arduino-cli, Local-tier PC tests and Wokwi in GitHub Actions; Node.js backend on a DigitalOcean Droplet.
- Firmware logic lives in the sketch's `src/` folder as plain C++ with no `Arduino.h` (the hardware-boundary absolute), so it can be unit-tested on a PC.
- Simulation: each unit is simulated separately. The radio goes behind a transport boundary (a serial stand-in in Wokwi), and audio is simulated as a serial "PLAY …" line. Real ESP-NOW and real audio are tested at the Real tier.
- Sound delivery: the Indoor Ringer relays files to the Doorbell over ESP-NOW; the Doorbell never uses Wi-Fi.
- Fallback: both units play the built-in sound whenever the selected file isn't complete and intact; never a previous custom sound.
- Book appendices: Ron will supply Appendix A and Appendix B. Appendix A gets **merged**: Ron's version is the base, with the new increment-card fields plus the spike card and Real-tier procedure added; Claude's draft appendix is dropped. Appendix B is inserted as-is and checked against Chapter 8's updated toolbox.
