"""Shared helpers for the FIRM absolute checks. Standard library only (absolute A-08)."""
import argparse
import os
import re
import sys

INCLUDE_RE = re.compile(r'^\s*#\s*include\s*([<"])([^>"]+)[>"]', re.MULTILINE)
SOURCE_EXT = (".h", ".hpp", ".c", ".cpp", ".cc", ".ino")


def parse_root(description):
    p = argparse.ArgumentParser(description=description)
    p.add_argument("--root", default=os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
                   help="repository root (default: the repo this script lives in)")
    return p.parse_args().root


def source_files(directory):
    """All C/C++/sketch files under a directory, sorted for stable output."""
    found = []
    if not os.path.isdir(directory):
        return found
    for base, _dirs, files in os.walk(directory):
        for name in files:
            if name.endswith(SOURCE_EXT):
                found.append(os.path.join(base, name))
    return sorted(found)


def strip_comments(text):
    """Remove // and /* */ comments (and keep line count) so rule text in comments isn't flagged."""
    def blank(m):
        return re.sub(r"[^\n]", " ", m.group(0))
    text = re.sub(r"/\*.*?\*/", blank, text, flags=re.DOTALL)
    text = re.sub(r"//[^\n]*", blank, text)
    return text


def read(path):
    with open(path, "r", encoding="utf-8", errors="replace") as f:
        return f.read()


def unit_dirs(root):
    """firmware/<unit> folders (doorbell, indoor-ringer, ...), excluding 'common'."""
    fw = os.path.join(root, "firmware")
    if not os.path.isdir(fw):
        return []
    return sorted(os.path.join(fw, d) for d in os.listdir(fw)
                  if os.path.isdir(os.path.join(fw, d)) and d != "common")


def finish(check_id, violations, ok_message):
    rel = lambda s: s  # messages are already relative
    if violations:
        print(f"FAIL {check_id}")
        for v in violations:
            print(f"  - {rel(v)}")
        sys.exit(1)
    print(f"PASS {check_id}: {ok_message}")
    sys.exit(0)
