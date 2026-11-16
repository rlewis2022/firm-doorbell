#!/usr/bin/env python3
"""Copy the shared code in firmware/common/logic/ into every unit (DL-037).

The Arduino IDE only compiles files inside a sketch's own folder, so each unit gets an exact copy at
firmware/<unit>/src/logic/common/. Edit only the master in firmware/common/logic/, then run this.
The gate's COMMON check (checks/check_common_copies.py) fails if a copy differs or is missing.
Standard library only (A-08).

Usage:  python3 tools/sync_common.py
"""
import os
import shutil
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def main():
    master = os.path.join(ROOT, "firmware", "common", "logic")
    if not os.path.isdir(master):
        sys.exit("No firmware/common/logic/ folder.")
    fw = os.path.join(ROOT, "firmware")
    units = sorted(d for d in os.listdir(fw) if d != "common" and os.path.isdir(os.path.join(fw, d)))
    for unit in units:
        dest = os.path.join(fw, unit, "src", "logic", "common")
        if os.path.isdir(dest):
            shutil.rmtree(dest)
        shutil.copytree(master, dest, ignore=shutil.ignore_patterns(".*"))  # skip .gitkeep and other dotfiles
        print(f"copied firmware/common/logic/ -> {os.path.relpath(dest, ROOT)}")


if __name__ == "__main__":
    main()
