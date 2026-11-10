#!/usr/bin/env python3
"""A-08: No dependency is added without a decision-log entry, and every version is pinned.

Why: every dependency is code you maintain; pinned versions make baselines comparable.
Scope: firmware sketch profiles (sketch.yaml), the gate workflow, the vendored doctest header,
backend/package.json (once it exists).
How: everything declared must appear in checks/approved_dependencies.json with the same version.
"""
import hashlib
import json
import os
import re
from common import finish, parse_root, read, unit_dirs


def main():
    root = parse_root(__doc__.splitlines()[0])
    approved = json.loads(read(os.path.join(root, "checks", "approved_dependencies.json")))
    v = []

    # 1. Arduino platforms and libraries in each sketch profile
    for unit in unit_dirs(root):
        y = os.path.join(unit, "sketch.yaml")
        if not os.path.isfile(y):
            continue
        rel = os.path.relpath(y, root)
        text = read(y)
        for m in re.finditer(r"^\s*-\s*platform:\s*([\w.-]+:[\w.-]+)\s*(?:\(([^)]*)\))?", text, re.M):
            name, ver = m.group(1), m.group(2)
            want = approved["arduino_platforms"].get(name)
            if want is None:
                v.append(f"{rel}: platform {name} is not approved")
            elif ver != want:
                v.append(f"{rel}: platform {name} pinned to {ver!r}, approved {want!r}")
        in_libs = False
        for line in text.splitlines():
            if re.match(r"^\s*libraries:\s*$", line):
                in_libs = True
                continue
            if in_libs:
                lm = re.match(r"^\s*-\s*(.+?)\s*\(([^)]*)\)\s*$", line)
                if lm:
                    name, ver = lm.group(1), lm.group(2)
                    want = approved["arduino_libraries"].get(name)
                    if want is None:
                        v.append(f"{rel}: library {name} is not approved")
                    elif ver != want:
                        v.append(f"{rel}: library {name} pinned to {ver!r}, approved {want!r}")
                elif re.match(r"^\s*-\s*\S", line):
                    v.append(f"{rel}: library entry without a pinned version: {line.strip()}")
                elif line.strip() and not line.startswith(" " * 6):
                    in_libs = False

    # 2. GitHub Actions: pinned to an approved commit SHA
    wf_dir = os.path.join(root, ".github", "workflows")
    for name in sorted(os.listdir(wf_dir)) if os.path.isdir(wf_dir) else []:
        rel = f".github/workflows/{name}"
        text = read(os.path.join(wf_dir, name))
        for m in re.finditer(r"uses:\s*([\w.-]+/[\w.-]+)@([\w.-]+)", text):
            action, ref = m.group(1), m.group(2)
            want = approved["github_actions"].get(action)
            if want is None:
                v.append(f"{rel}: action {action} is not approved")
            elif ref != want:
                v.append(f"{rel}: action {action}@{ref} is not the approved SHA {want}")
        for tool, env in (("arduino-cli", "ARDUINO_CLI_VERSION"), ("gitleaks", "GITLEAKS_VERSION")):
            for m in re.finditer(rf"{env}:\s*\"?([\w.]+)\"?", text):
                if m.group(1) != approved["tools"][tool]["version"]:
                    v.append(f"{rel}: {env}={m.group(1)}, approved {approved['tools'][tool]['version']}")
            for m in re.finditer(rf"{env.replace('VERSION', 'SHA256')}:\s*\"?([0-9a-f]+)\"?", text):
                key = [k for k in approved["tools"][tool] if k.startswith("sha256")][0]
                if m.group(1) != approved["tools"][tool][key]:
                    v.append(f"{rel}: {tool} checksum doesn't match the approved one")

    # 3. Vendored doctest header
    hdr = os.path.join(root, "tests", "local", "third_party", "doctest.h")
    if os.path.isfile(hdr):
        with open(hdr, "rb") as f:
            digest = hashlib.sha256(f.read()).hexdigest()
        if digest != approved["tools"]["doctest"]["sha256_header"]:
            v.append("tests/local/third_party/doctest.h is not the approved doctest version")

    # 4. Backend npm packages (none yet)
    pkg = os.path.join(root, "backend", "package.json")
    if os.path.isfile(pkg):
        data = json.loads(read(pkg))
        for section in ("dependencies", "devDependencies"):
            for name, ver in data.get(section, {}).items():
                want = approved["npm"].get(name)
                if want is None:
                    v.append(f"backend/package.json: {name} is not approved")
                elif ver != want:
                    v.append(f"backend/package.json: {name} is {ver!r}, approved exact pin {want!r}")

    finish("A-08", v, "all declared dependencies are approved and pinned")


if __name__ == "__main__":
    main()
