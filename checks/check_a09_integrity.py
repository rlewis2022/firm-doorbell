#!/usr/bin/env python3
"""A-09 / gate step 1: the walls guard themselves.

Why: a check the AI can change isn't a wall (FIRM Principle 14).
Scope: the protected set (PROTECTED_GLOBS below): absolute checks, the regression suite and locked
acceptance checks, tool configs and pins, the instruction file, the pipeline definition.
How: checks/manifest.sha256 lists every protected file with its SHA-256. On every gate run the
fingerprints are recomputed; any changed, missing or unlisted protected file fails the gate.

Changing a wall is allowed, but only deliberately: in its own commit, outside any increment, made
by the owner (Verifier for regression tests, Architect for absolute checks), running
    python3 checks/check_a09_integrity.py --update
so the manifest changes in the same commit and the change is visible in review.

Honest limit: the manifest is a file too. Working solo, this makes tampering visible at review;
it doesn't make it impossible. Branch protection on main is the anchor.
"""
import argparse
import fnmatch
import hashlib
import os
import sys

PROTECTED_GLOBS = [
    "CLAUDE.md",
    ".github/workflows/*",
    "checks/*.py",
    "checks/*.json",
    "checks/selftest/*",
    "tools/*.py",
    "firmware/*/sketch.yaml",
    "tests/local/*",
    "tests/local/locked/*",
    "tests/local/third_party/*",
    "tests/sim/*",
    "tests/real/*",
]
MANIFEST = "checks/manifest.sha256"


def protected_files(root):
    found = []
    for base, dirs, files in os.walk(root):
        dirs[:] = [d for d in dirs if d != ".git"]
        for name in files:
            rel = os.path.relpath(os.path.join(base, name), root).replace(os.sep, "/")
            if rel == MANIFEST:
                continue
            if any(fnmatch.fnmatchcase(rel, g) and rel.count("/") == g.count("/") for g in PROTECTED_GLOBS):
                found.append(rel)
    return sorted(found)


def sha256(path):
    with open(path, "rb") as f:
        return hashlib.sha256(f.read()).hexdigest()


def main():
    p = argparse.ArgumentParser(description="A-09 integrity check")
    p.add_argument("--root", default=os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    p.add_argument("--update", action="store_true", help="rewrite the manifest (wall-change commits only)")
    a = p.parse_args()
    root, mpath = a.root, os.path.join(a.root, MANIFEST)
    current = {rel: sha256(os.path.join(root, rel)) for rel in protected_files(root)}

    if a.update:
        with open(mpath, "w", encoding="utf-8", newline="\n") as f:
            for rel in sorted(current):
                f.write(f"{current[rel]}  {rel}\n")
        print(f"manifest updated: {len(current)} protected file(s). Commit this as a wall change, on its own.")
        return 0

    if not os.path.isfile(mpath):
        print(f"FAIL A-09: {MANIFEST} is missing")
        return 1
    listed = {}
    with open(mpath, encoding="utf-8") as f:
        for line in f:
            if line.strip():
                digest, rel = line.rstrip("\n").split("  ", 1)
                listed[rel] = digest
    v = []
    for rel, digest in sorted(listed.items()):
        if rel not in current:
            v.append(f"{rel}: protected file is missing")
        elif current[rel] != digest:
            v.append(f"{rel}: changed but the manifest wasn't updated in a wall-change commit")
    for rel in sorted(set(current) - set(listed)):
        v.append(f"{rel}: new protected file not in the manifest")
    if v:
        print("FAIL A-09 (integrity): the walls changed")
        for x in v:
            print(f"  - {x}")
        print("If this change is intended, it belongs in its own wall-change commit with --update (see CLAUDE.md).")
        return 1
    print(f"PASS A-09 (integrity): {len(listed)} protected file(s) match the manifest")
    return 0


if __name__ == "__main__":
    sys.exit(main())
