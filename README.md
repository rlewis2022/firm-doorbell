# firm-doorbell

A two-unit ESP32-S3 doorbell, built step by step as the worked example for **FIRM: Human-Led Software
Engineering with AI** by R.A. Lewis.

- **Doorbell** (outside, battery): press the button, it plays a sound and rings the Indoor Ringer.
- **Indoor Ringer** (inside, USB power, louder): rings when the Doorbell is pressed.
- They talk over **ESP-NOW**, so the bell works with no Wi-Fi or internet at all.
- Later: a small web backend to upload and select sounds, relayed to both units.

**FIRM** = **F**ramed · **I**ncremental · **R**egression-protected · **M**easured. Every decision, spike
and AI iteration is recorded in [`docs/firm/`](docs/firm/), and every change passes the machine gate in
[`.github/workflows/gate.yml`](.github/workflows/gate.yml).

## Status
**Baseline 0: the mold is in place** — folder layout, absolute checks, pinned build, Local-tier test
runner, protected-file manifest, and the AI instruction file ([`CLAUDE.md`](CLAUDE.md)). No product
behavior yet; the first increment (Doorbell plays its built-in sound on a button press) comes after
the Mold gate.

## Hardware
ESP32-S3 SuperMini ×2 · MAX98357A I2S amplifier ×2 · 8 Ω speakers · push button · 1S 3.7 V LiPo
(Doorbell). Wiring: see [`CLAUDE.md`](CLAUDE.md#hardware-facts-measured--discover-d1-spike-s-01).

## Run the gate locally
```bash
python3 checks/check_a09_integrity.py      # 1  integrity
python3 checks/selftest/run_selftests.py   #    the checks can still fail
python3 checks/run_absolutes.py            # 2  absolutes
make -C tests/local test                   # 3+4 Local-tier tests
```
Firmware builds with `arduino-cli compile --profile doorbell firmware/doorbell` (pinned in
`firmware/doorbell/sketch.yaml`), or open `firmware/doorbell/doorbell.ino` in the Arduino IDE
with the settings listed there.

## License
[MIT](LICENSE) © 2026 R.A. Lewis. Third-party code keeps its own license: doctest
(`tests/local/third_party/doctest.h`) is MIT, © Viktor Kirilov.
