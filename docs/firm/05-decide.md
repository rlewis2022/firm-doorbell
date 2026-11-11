# ESP32 Doorbell — Elaboration: Decide (FIRM)

Status: **APPROVED by Ron 2026-11-09 (Q1–Q7; decision log DL-011 to DL-017).** Next: put the mold in place (repo skeleton, check scripts, pipeline, manifest, instruction file), then the Mold gate.

Decide turns the evidence from Discover and spike S-01 into the **mold**: architecture, absolutes with checks, budgets, the repo and pipeline, and the ranked backlog. The Doorbell is detailed now (DL-004: Doorbell first). The Indoor Ringer, relay and backend are outlined, and get detailed after spike S-02.

---

## 1. Architecture

### 1.1 The system (outline)
```
 [Doorbell]  --ESP-NOW (versioned)-->  [Indoor Ringer]  --Wi-Fi/HTTPS (versioned)-->  [Backend on Droplet]  <--browser (login)-- [User]
  battery, no Wi-Fi                      USB power, Wi-Fi                               Node.js
  button, speaker                        speaker, config button                         upload/select sounds
```
- Every unit rings from its own flash, with nothing else reachable (Inception).
- The Doorbell never joins Wi-Fi. Sounds reach it only by relay from the Indoor Ringer (Inception, DL-004).

### 1.2 Doorbell firmware: three layers
| Layer | What it is | Rules |
|---|---|---|
| **Logic** (`src/logic/`) | Plain C++ that decides what happens: on wake → which sound to play → then sleep. Later: which sound is selected, and whether a stored file passed verification. | No Arduino or ESP headers at all, so it **compiles and is tested on a PC** (Local tier). It talks to hardware only through the interfaces below. |
| **Hardware access** (`src/hw/`) | Thin wrappers around the ESP32 functions proven in S-01: I2S `playWAV()` (DL-007), deep sleep and button wake on GPIO2 (DL-006). Later: ESP-NOW, file storage. | No decisions here, just "do this to the hardware." Tested at the Simulated/Real tiers. |
| **Sketch** (`doorbell.ino`) | Builds the hardware objects, hands them to the logic, runs it once per wake. | As short as possible. No logic. |

**Interfaces the logic sees** (names are proposals):
- `AudioOut::play(const uint8_t* wav, size_t len)`: plays a WAV from memory or flash.
- `PowerCtl::wakeReason()` and `PowerCtl::sleepUntilButton()`.
- Later: `SoundStore` (verified custom sound, if any), `Link` (ESP-NOW messages).

**The first behavior, in plain terms:** on a button wake, play the selected sound (for now, always the built-in one), then sleep. **Sound comes first**; nothing (logging, radio, storage) runs before playback starts.

**The built-in sound** is a WAV converted at build time into a `const` array in flash (DL-008), at 24 kHz (DL-010), 16-bit, mono. A small script in the repo does the conversion, so the sound file itself is the source and the array is generated, never hand-edited.

### 1.3 Indoor Ringer and backend (outline only)
- **Indoor Ringer:** the same three layers. Adds Wi-Fi, a backend client, file download and verification, and the ESP-NOW relay to the Doorbell. Detailed after S-02.
- **Backend:** Node.js on a $4–6 Droplet (Discover D6). Login (single user/family), upload, select, versioned API. Detailed in a later sprint.

### 1.4 Open questions in the architecture
- **A1. Battery power path:** on battery, the SuperMini's 5V pin probably isn't powered, so the MAX98357A may need power from the battery side. To be checked against the board before the battery increment.
- **A2. Flash layout for the Doorbell:** the Doorbell can't receive over-the-air updates (no Wi-Fi), so the "No OTA" layout (2 MB app / 1.875 MB storage) gives more room than the default (1.25 MB app ×2 / 1.375 MB storage). The proposal is No OTA for the Doorbell, with the Indoor Ringer decided later.

---

## 2. Construction absolutes

Format per the book: rule · why · scope · check · owner. Owner is Ron (Architect) for all of them. **Bold** checks run on every iteration (Local tier); the rest run at increment transition.

| # | Rule | Why | Scope | Check |
|---|---|---|---|---|
| **A-01** | No secrets are ever committed: no Wi-Fi credentials, passwords, keys or tokens. | The repo is public from day one (Inception). | Whole repo | **Secret scanner in the gate** (tool to be chosen — Q3) |
| **A-02** | Logic code never includes Arduino, ESP-IDF or hardware headers. | Keeps decisions testable on a PC, and keeps hardware the one thing that's easy to change (Parnas; book Ch. 7). | `src/logic/` in both firmwares | **Dependency script fails on any banned include; logic must compile with the PC compiler** |
| **A-03** | The Doorbell never connects to an access point and holds no Wi-Fi credentials. *(ESP-NOW still needs the radio started in station mode; that's allowed.)* | Battery life, and no secrets on a device mounted outdoors (Inception, DL-004). | Doorbell firmware | **Script fails on `WiFi.begin`, `esp_wifi_connect`, `softAP`, or any SSID/password constant in the Doorbell code** |
| **A-04** | Neither unit plays a sound file it hasn't verified as complete and intact. If verification fails, it plays the built-in sound, never an older custom sound. | Inception decisions A–C: the built-in sound is an unmistakable signal that the new sound didn't arrive. | Both firmwares (applies once custom sounds exist) | Local tests: missing, partial, corrupted and wrong-version files all → built-in sound |
| **A-05** | Ringing never waits on the network, the backend, the radio link or a file transfer. Sound starts first. | The core promise: rings with the network down (Inception). | Both firmwares | **Dependency script: the ring path can't include network/transfer modules.** Local test: with a fake link that hangs or fails, the sound is still requested first |
| **A-06** | The built-in sound is a `const` array generated from a source WAV, stored in firmware flash, never computed at runtime or read from the file system. | S-01: computing it took 237–316 ms per wake (DL-008); flash can't be lost or corrupted by a bad transfer. | Both firmwares | **Script checks the generated array matches its source WAV** (so nobody hand-edits it) |
| **A-07** | Every message between separately updated parts carries a contract version (major.minor); mismatches fail safe. | Book Fix 6: the Doorbell and Ringer won't always run the same firmware. | ESP-NOW and backend contracts (from the first message) | Contract tests with golden samples of each supported version |
| **A-08** | No library, tool or dependency is added without a decision-log entry, and all versions are pinned (Arduino ESP32 core 3.3.12, arduino-cli, Node packages). | Every dependency is code you maintain; pinned versions make baselines comparable. | Whole repo | **Script compares declared dependencies with the approved list** |
| **A-09** | The walls guard themselves: absolute checks, the regression suite, tool configs, the instruction file and the pipeline definition can't change in an AI iteration. | Book Fix 1. | Whole repo | **Gate step 1: protected files match the manifest** |

**Not checkable by a tool → human review:** "the sketch contains no logic" (A-02's spirit) gets a Verifier checklist item.

---

## 3. Budgets
| # | Budget | Limit (proposed) | Warning (80%) | How measured / tier |
|---|---|---|---|---|
| B-01 | Doorbell firmware image size | **≤ 75% of the app partition** (1,572,864 bytes with No OTA; 983,040 with the default layout) | at 80% of the limit | From arduino-cli build output, **every iteration** (Local) |
| B-02 | Sound file format and size | **24 kHz, 16-bit, mono PCM WAV, ≤ 3.0 s → ≤ 144,044 bytes** | ≥ 2.4 s | Backend rejects anything else at upload; device verifies on receipt |
| B-03 | Press-to-sound time (Doorbell) | **≤ 300 ms** (proposed; DL-008) | 240 ms | Real tier: phone-recording procedure at increment transition |
| B-04 | Doorbell average current | **≤ 0.9 mA average** (derived: 1000 mAh ÷ 720 h ÷ 1.5 margin, with the battery Ron is considering) | 0.72 mA | Real tier: µA multimeter procedure (DL-009) |

Context for B-02: one 3-second sound is ≈ 98 ESP-NOW packets at 1,470 bytes. Each unit only needs to store **one** custom sound (the selected one); the built-in sound lives in firmware flash. Storage space is 1,441,792 bytes (default layout) or 1,966,080 bytes (No OTA), so about 9 or 13 full-length sounds before file-system overhead. One is plenty.

---

## 4. Repo layout and pipeline (proposal) — repo: `rlewis2022/firm-doorbell`
```
/firmware/doorbell/            doorbell.ino, src/logic/, src/hw/, src/sounds/ (generated)
/firmware/indoor-ringer/       (later)
/firmware/common/              shared message definitions (versioned), shared logic
/backend/                      (later)
/sounds/                       source WAVs (built-in sound)
/tests/local/                  PC tests for logic (every iteration)
/tests/sim/                    Wokwi scenarios (increment transition)
/tests/real/                   written Real-tier procedures + recorded results
/checks/                       absolute-check scripts + protected-set manifest
/spikes/                       archived spikes (read-only, never merged)
/docs/firm/                    inception, discover, decide, decision log, run log
/.github/workflows/gate.yml    the machine gate
```
**The machine gate (every push):** 1) integrity check → 2) absolutes (A-01, A-02, A-03, A-05, A-06, A-08) + static budget B-01 via an arduino-cli build → 3) acceptance checks for the current increment → 4) regression suite (Local tests).
**Increment transition:** Wokwi scenarios (≤ 50 free CI minutes a month), then Real-tier procedures on the bench.

---

## 5. Ranked backlog (Pareto: the vital few first)
The **vital few** is "it rings, inside and out, with the network down." That's the first three items. Everything after them adds configurability.

| Rank | Item | Unit | Needs |
|---|---|---|---|
| 1 | Button wake plays the built-in sound from flash, then sleeps | Doorbell | Pipeline + Local tests set up first |
| 2 | Indoor Ringer plays the built-in sound on a ring message (and on its own test button) | Indoor Ringer | — |
| 3 | Doorbell sends a **ring** message over ESP-NOW (contract v1.0); Ringer rings | Both | ESP-NOW only, no Wi-Fi, so **S-02 isn't needed yet** |
| 4 | Battery power path, status/charging LED, sleep-current measurement | Doorbell | Meter (DL-009), A1 |
| 5 | Indoor Ringer joins Wi-Fi and reads its config from the backend | Ringer + backend | **Spike S-02** (channel sharing) |
| 6 | Backend: login, upload, select a sound | Backend | Droplet |
| 7 | Ringer downloads, verifies and plays the selected sound (fallback per A-04) | Ringer | — |
| 8 | Ringer relays the sound to the Doorbell over ESP-NOW; Doorbell verifies and plays it | Both | S-02 results |

**Against the 2026-11-17 date:** ranks 1–3 (plus the pipeline) are a realistic target. That's a working two-unit doorbell with no network at all. Ranks 4–8 are a second sprint. **This is a Product Owner call** (Q6).

---

## 6. Questions for Ron (Product Owner / Architect)
- **Q1.** Approve the three-layer architecture (logic / hardware access / sketch)?
- **Q2.** Approve absolutes A-01 to A-09? Add, change or drop any?
- **Q3.** The secret scanner and the PC test framework are new tools (A-08). Proposal: **gitleaks** for secrets, and **doctest** (a single header file, MIT license) for the PC tests. Or a zero-dependency option: simple asserts plus a tiny runner script.
- **Q4.** The Doorbell flash layout: **No OTA** (more room, A2)?
- **Q5.** Approve budgets B-01 to B-04, especially the 3-second cap on sounds?
- **Q6.** Sprint 1 (to 11-17) = backlog ranks 1–3 + pipeline; ranks 4–8 in sprint 2?
- **Q7.** ~~The GitHub repo name.~~ **Answered 2026-11-09: `rlewis2022/firm-doorbell`** (public; https://github.com/rlewis2022/firm-doorbell). Nothing will be pushed until the Mold gate passes.
