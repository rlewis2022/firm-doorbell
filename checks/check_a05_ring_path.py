#!/usr/bin/env python3
"""A-05: Ringing never waits on the network, the backend, the radio link or a file transfer.

Why: the core promise is that it rings with the network down (Inception).
Scope: the ring path, by convention firmware/*/src/logic/ring/ (see CLAUDE.md).
How (structural half): no ring-path file may include link, transfer, network or backend code, or
any hardware header. The behavioral half is a Local test in each increment that touches ringing:
with a fake link that hangs or fails, the sound is still requested first.
"""
import os
import re
from common import INCLUDE_RE, finish, parse_root, read, source_files, unit_dirs

FORBIDDEN = re.compile(
    r"(^|/)(link|transfer|relay|net|network|backend|wifi|espnow|esp_now|http)(/|_|\.|$)|WiFi|esp_now|HTTP",
    re.IGNORECASE,
)


def main():
    root = parse_root(__doc__.splitlines()[0])
    violations, checked = [], 0
    for unit in unit_dirs(root):
        ring = os.path.join(unit, "src", "logic", "ring")
        for path in source_files(ring):
            checked += 1
            text = read(path)
            for m in INCLUDE_RE.finditer(text):
                name = m.group(2)
                if FORBIDDEN.search(name):
                    line = text.count("\n", 0, m.start()) + 1
                    violations.append(f"{os.path.relpath(path, root)}:{line} includes \"{name}\" - "
                                      f"the ring path can't depend on link/network/transfer code (A-05)")
    finish("A-05", violations, f"{checked} ring-path file(s) are independent of link/network/transfer code")


if __name__ == "__main__":
    main()
