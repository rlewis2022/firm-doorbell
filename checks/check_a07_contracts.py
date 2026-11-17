#!/usr/bin/env python3
"""A-07: Every message between separately updated parts carries a contract version (major.minor).

Why: the two units (and later the backend) won't always run the same firmware (DL-012; Fix 6).
Scope: message formats in firmware/common/logic/contract/ (one header per message family).
How: each contract header must declare `kMajor` and `kMinor` as numbers, and a golden sample of that
exact version must exist at tests/local/golden/<header name>_<major>_<minor>.bin (protected, A-09).
The locked Local tests decode the golden samples, so a changed format can't slip through.
"""
import os
import re
from common import finish, parse_root, read, strip_comments

MAJOR = re.compile(r"\bkMajor\s*=\s*(\d+)")
MINOR = re.compile(r"\bkMinor\s*=\s*(\d+)")


def main():
    root = parse_root(__doc__.splitlines()[0])
    cdir = os.path.join(root, "firmware", "common", "logic", "contract")
    violations, checked = [], 0
    if os.path.isdir(cdir):
        for name in sorted(os.listdir(cdir)):
            if not name.endswith(".h"):
                continue
            checked += 1
            rel = os.path.relpath(os.path.join(cdir, name), root)
            text = strip_comments(read(os.path.join(cdir, name)))
            ma, mi = MAJOR.search(text), MINOR.search(text)
            if not ma or not mi:
                violations.append(f"{rel}: doesn't declare its version as kMajor and kMinor (A-07)")
                continue
            stem = name[:-2]
            golden = os.path.join(root, "tests", "local", "golden", f"{stem}_{ma.group(1)}_{mi.group(1)}.bin")
            if not os.path.isfile(golden) or os.path.getsize(golden) == 0:
                violations.append(f"{rel}: version {ma.group(1)}.{mi.group(1)} has no golden sample at "
                                  f"{os.path.relpath(golden, root)} (A-07)")
    finish("A-07", violations, f"{checked} contract(s) declare major.minor and have a golden sample")


if __name__ == "__main__":
    main()
