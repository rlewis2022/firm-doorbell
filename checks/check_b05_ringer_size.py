#!/usr/bin/env python3
"""B-05: Indoor Ringer firmware image <= 1,572,864 bytes (75% of the 2 MB No-OTA app partition).

Why: an AI that over-generates adds bytes, and flash is a hard limit (DL-027).
How: reads the arduino-cli compile log. It also confirms the build used the No-OTA layout
(maximum program storage 2,097,152 bytes), so a changed partition setting can't hide growth.
Warning line: 80% of the limit (flagged, not failed).
Usage: python3 checks/check_b05_ringer_size.py build.log
"""
import re
import sys

LIMIT = 1_572_864
WARN = int(LIMIT * 0.8)
NO_OTA_MAX = 2_097_152


def main(argv):
    if len(argv) != 2:
        print(__doc__)
        return 2
    with open(argv[1], encoding="utf-8", errors="replace") as f:
        log = f.read()
    m = re.search(r"Sketch uses (\d+) bytes .*?Maximum is (\d+) bytes", log, re.S)
    if not m:
        print("FAIL B-05: couldn't find the program size in the build log (did the build fail?)")
        return 1
    used, maximum = int(m.group(1)), int(m.group(2))
    if maximum != NO_OTA_MAX:
        print(f"FAIL B-05: build used a {maximum}-byte app partition; the Indoor Ringer must use No OTA ({NO_OTA_MAX})")
        return 1
    pct = 100.0 * used / LIMIT
    if used > LIMIT:
        print(f"FAIL B-05: firmware is {used} bytes, limit {LIMIT} ({pct:.1f}% of budget)")
        return 1
    if used >= WARN:
        print(f"::warning title=B-05 budget::firmware is {used} bytes, {pct:.1f}% of the {LIMIT}-byte budget (warning line 80%)")
    print(f"PASS B-05: firmware is {used} bytes, {pct:.1f}% of the {LIMIT}-byte budget")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
