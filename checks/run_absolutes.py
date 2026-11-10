#!/usr/bin/env python3
"""Gate step 2: run every tool-checked absolute that needs only the source tree.

A-01 (gitleaks) and B-01 (firmware size) run as their own gate steps because they need a tool
or a build. Exit code is non-zero if any absolute fails. Run locally before every push:
    python3 checks/run_absolutes.py
"""
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
CHECKS = [
    "check_a02_logic_includes.py",
    "check_a03_doorbell_no_wifi.py",
    "check_a05_ring_path.py",
    "check_a06_builtin_sound.py",
    "check_a08_dependencies.py",
]


def main():
    extra = sys.argv[1:]
    failed = []
    for c in CHECKS:
        r = subprocess.run([sys.executable, os.path.join(HERE, c), *extra])
        if r.returncode != 0:
            failed.append(c)
    if failed:
        print(f"\nABSOLUTES FAILED: {', '.join(failed)}")
        return 1
    print("\nAll source-tree absolutes pass.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
