#!/usr/bin/env python3
"""A-02: Logic code never includes Arduino, ESP-IDF or hardware headers.

Why: keeps decisions testable on a PC (Local tier) and hardware the one thing that's easy to change.
Scope: firmware/*/src/logic/ and firmware/common/logic/.
How: allow-list. A logic file may include only C/C++ standard headers (<...>) or other logic files
("..." that resolve inside a logic folder). Anything else fails. The PC build in tests/local is the
second half of this check: logic that needs hardware won't compile there.
"""
import os
from common import INCLUDE_RE, finish, parse_root, read, source_files, unit_dirs

STD_HEADERS = {
    # C++ standard library
    "algorithm", "array", "atomic", "bitset", "cassert", "cctype", "cerrno", "cfloat", "chrono",
    "cinttypes", "climits", "cmath", "cstdarg", "cstddef", "cstdint", "cstdio", "cstdlib", "cstring",
    "deque", "functional", "initializer_list", "iterator", "limits", "list", "map", "memory", "new",
    "numeric", "optional", "queue", "set", "span", "stack", "string", "string_view", "tuple",
    "type_traits", "unordered_map", "unordered_set", "utility", "variant", "vector",
    # C headers commonly used from C++
    "assert.h", "ctype.h", "float.h", "inttypes.h", "limits.h", "math.h", "stdbool.h", "stddef.h",
    "stdint.h", "stdio.h", "stdlib.h", "string.h",
}


def logic_roots(root):
    roots = [os.path.join(u, "src", "logic") for u in unit_dirs(root)]
    roots.append(os.path.join(root, "firmware", "common", "logic"))
    return [r for r in roots if os.path.isdir(r)]


def main():
    root = parse_root(__doc__.splitlines()[0])
    roots = logic_roots(root)
    violations, checked = [], 0
    for lroot in roots:
        for path in source_files(lroot):
            checked += 1
            text = read(path)
            for m in INCLUDE_RE.finditer(text):
                kind, name = m.group(1), m.group(2)
                line = text.count("\n", 0, m.start()) + 1
                where = f"{os.path.relpath(path, root)}:{line}"
                if kind == "<":
                    if name not in STD_HEADERS:
                        violations.append(f"{where} includes <{name}> - not a standard header (A-02)")
                else:
                    target = os.path.normpath(os.path.join(os.path.dirname(path), name))
                    inside = any(target.startswith(os.path.normpath(r) + os.sep) for r in roots)
                    if not (inside and os.path.isfile(target)):
                        violations.append(f'{where} includes "{name}" - not a file inside a logic folder (A-02)')
    finish("A-02", violations, f"{checked} logic file(s) include only standard headers and other logic files")


if __name__ == "__main__":
    main()
