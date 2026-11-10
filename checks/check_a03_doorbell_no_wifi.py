#!/usr/bin/env python3
"""A-03: The Doorbell never connects to an access point and holds no Wi-Fi credentials.

Why: battery life, and no secrets on a device mounted outdoors (Inception; DL-004).
Scope: firmware/doorbell/ (all code).
Allowed: starting the radio in station mode for ESP-NOW, e.g. WiFi.mode(WIFI_STA).
Banned: anything that joins or creates a network, or stores credentials.
Comments are ignored, so the rule can be explained in code without tripping the check.
"""
import os
import re
from common import finish, parse_root, read, source_files, strip_comments

BANNED = [
    (r"\bWiFi\s*\.\s*begin\s*\(", "WiFi.begin() joins a network"),
    (r"\bWiFi\s*\.\s*softAP\w*\s*\(", "softAP creates a network"),
    (r"\bWiFiMulti\b", "WiFiMulti joins networks"),
    (r"\besp_wifi_connect\s*\(", "esp_wifi_connect() joins a network"),
    (r"\besp_wifi_set_config\s*\(", "esp_wifi_set_config() sets network credentials"),
    (r"\b(WiFiClient|WiFiClientSecure|HTTPClient|WebServer|AsyncWebServer)\b", "network client/server on the Doorbell"),
    (r"#\s*include\s*[<\"](HTTPClient|WebServer|WiFiMulti|WiFiClientSecure)\.h[>\"]", "network library on the Doorbell"),
    (r"\b\w*(ssid|passphrase|wifi_?pass(word)?|password)\w*\s*(\[[^\]]*\])?\s*(=|:)", "a Wi-Fi credential constant"),
]


def main():
    root = parse_root(__doc__.splitlines()[0])
    doorbell = os.path.join(root, "firmware", "doorbell")
    violations, files = [], source_files(doorbell)
    for path in files:
        code = strip_comments(read(path))
        for pattern, why in BANNED:
            for m in re.finditer(pattern, code, flags=re.IGNORECASE):
                line = code.count("\n", 0, m.start()) + 1
                violations.append(f"{os.path.relpath(path, root)}:{line}: {why} (A-03)")
    finish("A-03", violations, f"{len(files)} Doorbell file(s) never join a network or hold credentials")


if __name__ == "__main__":
    main()
