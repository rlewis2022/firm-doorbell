#!/usr/bin/env python3
"""COMMON: each unit's copy of the shared code matches the master in firmware/common/ exactly.

Why: the Arduino IDE only compiles files inside a sketch's own folder, so shared code (message
contracts, A-07) is kept once in firmware/common/logic/ and copied into each unit by
tools/sync_common.py (DL-037). A copy edited by hand, or not refreshed, would let the two units
disagree about a message format.
How: for every file under firmware/common/logic/, each unit must have a byte-identical copy at
firmware/<unit>/src/logic/common/<same path>, and no unit may have files there that the master doesn't.
Fix a failure by editing the master and running: python3 tools/sync_common.py
"""
import filecmp
import os
from common import finish, parse_root, unit_dirs


def files_under(d):
    out = set()
    if os.path.isdir(d):
        for base, _dirs, names in os.walk(d):
            for n in names:
                if n.startswith("."):  # .gitkeep and other dotfiles aren't shared code
                    continue
                out.add(os.path.relpath(os.path.join(base, n), d))
    return out


def main():
    root = parse_root(__doc__.splitlines()[0])
    master = os.path.join(root, "firmware", "common", "logic")
    want = files_under(master)
    violations = []
    for unit in unit_dirs(root):
        copy_dir = os.path.join(unit, "src", "logic", "common")
        have = files_under(copy_dir)
        if not want and not have:
            continue
        for rel in sorted(want):
            c = os.path.join(copy_dir, rel)
            if not os.path.isfile(c):
                violations.append(f"{os.path.relpath(c, root)} is missing (run tools/sync_common.py)")
            elif not filecmp.cmp(os.path.join(master, rel), c, shallow=False):
                violations.append(f"{os.path.relpath(c, root)} differs from firmware/common/logic/{rel} - "
                                  f"edit the master, then run tools/sync_common.py")
        for rel in sorted(have - want):
            violations.append(f"{os.path.relpath(os.path.join(copy_dir, rel), root)} has no master in "
                              f"firmware/common/logic/ (run tools/sync_common.py)")
    finish("COMMON", violations, f"{len(want)} shared file(s) copied identically into every unit")


if __name__ == "__main__":
    main()
