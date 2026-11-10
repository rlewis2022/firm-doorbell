#!/usr/bin/env python3
"""Self-tests for the absolute checks: each check must PASS a clean tree and FAIL a violating one.

A check that can't fail proves nothing (FIRM: confirm each check fails first). These run in the
gate so a weakened check is caught, and they need only the Python standard library.
"""
import json
import os
import shutil
import struct
import subprocess
import sys
import tempfile

CHECKS_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
REPO = os.path.dirname(CHECKS_DIR)
results = []


def write(root, rel, text, mode="w"):
    path = os.path.join(root, rel)
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, mode) as f:
        f.write(text)


def wav(rate=24000, bits=16, ch=1, seconds=0.1):
    n = int(rate * seconds) * ch * (bits // 8)
    hdr = b"RIFF" + struct.pack("<I", 36 + n) + b"WAVE" + b"fmt " + struct.pack(
        "<IHHIIHH", 16, 1, ch, rate, rate * ch * bits // 8, ch * bits // 8, bits) + b"data" + struct.pack("<I", n)
    return hdr + b"\x00" * n


def base_tree():
    """A minimal clean tree: approved deps copied from the real repo, empty firmware folders."""
    root = tempfile.mkdtemp(prefix="firm-selftest-")
    os.makedirs(os.path.join(root, "checks"))
    shutil.copy(os.path.join(CHECKS_DIR, "approved_dependencies.json"), os.path.join(root, "checks"))
    for d in ("firmware/doorbell/src/logic/ring", "firmware/doorbell/src/hw",
              "firmware/doorbell/src/sounds", "firmware/common/logic", "sounds"):
        os.makedirs(os.path.join(root, d))
    write(root, "firmware/doorbell/src/logic/ring/ring.h", '#pragma once\n#include <stdint.h>\n#include "../audio_out.h"\n')
    write(root, "firmware/doorbell/src/logic/audio_out.h", "#pragma once\n#include <cstddef>\n")
    return root


def run(check, root, *args):
    r = subprocess.run([sys.executable, os.path.join(CHECKS_DIR, check), "--root", root, *args],
                       capture_output=True, text=True)
    return r.returncode


def expect(name, check, mutate, should_fail):
    root = base_tree()
    try:
        mutate(root)
        rc = run(check, root)
        ok = (rc != 0) if should_fail else (rc == 0)
        results.append((ok, f"{'FAILS' if should_fail else 'PASSES'} as expected: {name}" if ok
                        else f"WRONG RESULT: {name} (exit {rc})"))
    finally:
        shutil.rmtree(root)


def gen_header(root, wav_name, unit="doorbell"):
    sys.path.insert(0, os.path.join(REPO, "tools"))
    import wav2header
    data = open(os.path.join(root, "sounds", wav_name), "rb").read()
    out = os.path.join(root, "firmware", unit, "src", "sounds", wav_name.replace(".wav", ".h"))
    with open(out, "w", newline="\n") as f:
        f.write(wav2header.render_header(os.path.join(root, "sounds", wav_name), data))
    return out


def main():
    nothing = lambda r: None
    # A-02
    expect("A-02 clean logic", "check_a02_logic_includes.py", nothing, False)
    expect("A-02 logic includes Arduino.h", "check_a02_logic_includes.py",
           lambda r: write(r, "firmware/doorbell/src/logic/bad.cpp", "#include <Arduino.h>\n"), True)
    expect("A-02 logic includes a hw/ file", "check_a02_logic_includes.py",
           lambda r: (write(r, "firmware/doorbell/src/hw/i2s.h", "#pragma once\n"),
                      write(r, "firmware/doorbell/src/logic/bad.cpp", '#include "../hw/i2s.h"\n')), True)
    expect("A-02 logic includes esp_sleep.h", "check_a02_logic_includes.py",
           lambda r: write(r, "firmware/common/logic/bad.h", "#include <esp_sleep.h>\n"), True)
    # A-03
    expect("A-03 clean Doorbell (ESP-NOW station mode allowed)", "check_a03_doorbell_no_wifi.py",
           lambda r: write(r, "firmware/doorbell/src/hw/radio.cpp",
                           "// never call WiFi.begin here (A-03)\nvoid start(){ WiFi.mode(WIFI_STA); }\n"), False)
    expect("A-03 Doorbell calls WiFi.begin", "check_a03_doorbell_no_wifi.py",
           lambda r: write(r, "firmware/doorbell/doorbell.ino", 'void setup(){ WiFi.begin("x"); }\n'), True)
    expect("A-03 Doorbell holds a password constant", "check_a03_doorbell_no_wifi.py",
           lambda r: write(r, "firmware/doorbell/src/hw/cfg.h", 'const char* WIFI_PASSWORD = "x";\n'), True)
    expect("A-03 Doorbell calls esp_wifi_connect", "check_a03_doorbell_no_wifi.py",
           lambda r: write(r, "firmware/doorbell/src/hw/n.cpp", "void f(){ esp_wifi_connect(); }\n"), True)
    # A-05
    expect("A-05 clean ring path", "check_a05_ring_path.py", nothing, False)
    expect("A-05 ring path includes link code", "check_a05_ring_path.py",
           lambda r: write(r, "firmware/doorbell/src/logic/ring/ring.cpp", '#include "../link/link.h"\n'), True)
    expect("A-05 ring path includes ESP-NOW", "check_a05_ring_path.py",
           lambda r: write(r, "firmware/doorbell/src/logic/ring/ring.cpp", "#include <esp_now.h>\n"), True)
    # A-06
    def good_sound(r):
        write(r, "sounds/builtin.wav", wav(), "wb")
        gen_header(r, "builtin.wav")
    def edited_sound(r):
        good_sound(r)
        h = os.path.join(r, "firmware/doorbell/src/sounds/builtin.h")
        t = open(h).read().replace("0x00,", "0x01,", 1)
        open(h, "w").write(t)
    def wrong_rate(r):
        write(r, "sounds/builtin.wav", wav(rate=32000), "wb")
        gen_header(r, "builtin.wav")
    def too_long(r):
        write(r, "sounds/builtin.wav", wav(seconds=3.5), "wb")
        gen_header(r, "builtin.wav")
    expect("A-06 generated header matches its WAV", "check_a06_builtin_sound.py", good_sound, False)
    expect("A-06 hand-edited header", "check_a06_builtin_sound.py", edited_sound, True)
    expect("A-06 header not generated by the tool", "check_a06_builtin_sound.py",
           lambda r: write(r, "firmware/doorbell/src/sounds/x.h", "const unsigned char x[] = {0};\n"), True)
    expect("A-06 WAV at 32 kHz breaks B-02", "check_a06_builtin_sound.py", wrong_rate, True)
    expect("A-06 WAV longer than 3 s breaks B-02", "check_a06_builtin_sound.py", too_long, True)
    # A-08
    expect("A-08 approved platform pin", "check_a08_dependencies.py",
           lambda r: write(r, "firmware/doorbell/sketch.yaml",
                           "profiles:\n  doorbell:\n    fqbn: esp32:esp32:esp32s3\n    platforms:\n      - platform: esp32:esp32 (3.3.12)\n"), False)
    expect("A-08 different core version", "check_a08_dependencies.py",
           lambda r: write(r, "firmware/doorbell/sketch.yaml",
                           "profiles:\n  doorbell:\n    platforms:\n      - platform: esp32:esp32 (3.3.13)\n"), True)
    expect("A-08 unapproved library", "check_a08_dependencies.py",
           lambda r: write(r, "firmware/doorbell/sketch.yaml",
                           "profiles:\n  doorbell:\n    platforms:\n      - platform: esp32:esp32 (3.3.12)\n    libraries:\n      - ESP8266Audio (1.9.7)\n"), True)
    expect("A-08 unpinned GitHub action", "check_a08_dependencies.py",
           lambda r: write(r, ".github/workflows/gate.yml", "steps:\n  - uses: actions/checkout@v5\n"), True)
    expect("A-08 unapproved npm package", "check_a08_dependencies.py",
           lambda r: write(r, "backend/package.json", json.dumps({"dependencies": {"left-pad": "1.3.0"}})), True)
    # A-09 (integrity): clean after --update, then tamper
    def a09(name, tamper):
        root = base_tree()
        try:
            write(root, "CLAUDE.md", "rules\n")
            write(root, "tests/local/test_x.cpp", "// locked check\n")
            subprocess.run([sys.executable, os.path.join(CHECKS_DIR, "check_a09_integrity.py"),
                            "--root", root, "--update"], capture_output=True)
            if tamper:
                tamper(root)
            rc = run("check_a09_integrity.py", root)
            should_fail = tamper is not None
            ok = (rc != 0) if should_fail else (rc == 0)
            results.append((ok, f"{'FAILS' if should_fail else 'PASSES'} as expected: {name}" if ok
                            else f"WRONG RESULT: {name} (exit {rc})"))
        finally:
            shutil.rmtree(root)
    a09("A-09 untouched walls", None)
    a09("A-09 locked test edited", lambda r: write(r, "tests/local/test_x.cpp", "// loosened\n"))
    a09("A-09 instruction file edited", lambda r: write(r, "CLAUDE.md", "no rules\n"))
    a09("A-09 new unlisted protected file", lambda r: write(r, "tests/local/test_new.cpp", "//\n"))
    a09("A-09 locked test deleted", lambda r: os.remove(os.path.join(r, "tests/local/test_x.cpp")))

    bad = [m for ok, m in results if not ok]
    for ok, m in results:
        print(("ok   " if ok else "BAD  ") + m)
    print(f"\n{len(results) - len(bad)}/{len(results)} check self-tests behaved as expected")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
