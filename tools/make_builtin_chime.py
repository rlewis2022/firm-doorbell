#!/usr/bin/env python3
"""Generate the Doorbell's built-in sound: a two-strike bell "ding-dong" (sounds/builtin_dingdong.wav).

A supplemental tool (DL-022): nothing in the firmware or the gate runs it. It records how the
built-in sound was made, so the sound can be regenerated or adjusted on purpose. The WAV it writes
is the source that tools/wav2header.py turns into a const array in flash (absolute A-06).

Output meets budget B-02: PCM, 24,000 Hz, 16-bit, mono, 2.9 s (139,244 bytes).
Sound: a D5 "ding", then a B-flat4 "dong" (a major third down) 0.62 s later, with church-bell
partials (hum, prime, tierce, quint, nominal...), a short clapper knock, no pitch shimmer and no
reverb. The ding is damped when the dong strikes so their overtones can't beat ("wavy" sound).
Deterministic: the same output every run. Standard library only (absolute A-08).

Usage:  python3 tools/make_builtin_chime.py sounds/builtin_dingdong.wav
"""
import math
import random
import struct
import sys
import wave

SR = 24000
SECONDS = 2.9
N = int(SR * SECONDS)
DING_HZ, DONG_HZ, DONG_AT = 587.33, 466.16, 0.62
# (ratio to the strike note, amplitude, decay time in seconds)
PARTIALS = [(0.5, 0.30, 2.2), (1.0, 1.00, 1.5), (1.19, 0.40, 1.0), (1.5, 0.28, 0.8),
            (2.0, 0.50, 0.7), (2.5, 0.15, 0.45), (3.0, 0.08, 0.3)]
PEAK = 0.85            # about -1.4 dBFS
FADE_SECONDS = 0.35    # ends on true silence, no click


def bell(f0, start, gain, seed):
    rng = random.Random(seed)
    out = [0.0] * N
    s0 = int(round(start * SR))
    for i in range(s0, N):
        tt = (i - s0) / SR
        v = 0.0
        for r, a, d in PARTIALS:
            v += a * math.exp(-tt / d) * math.sin(2 * math.pi * f0 * r * tt)
        v += rng.gauss(0.0, 1.0) * math.exp(-tt / 0.004) * 0.2     # clapper knock
        v *= min(tt / 0.003, 1.0)                                  # 3 ms attack
        out[i] = v * gain
    return out


def main():
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    ding = bell(DING_HZ, 0.0, 1.0, seed=7)
    dong = bell(DONG_HZ, DONG_AT, 1.05, seed=8)
    s_dong = int(round(DONG_AT * SR))
    y = []
    for i in range(N):
        d = ding[i]
        if i >= s_dong:
            d *= math.exp(-((i - s_dong) / SR) / 0.04)            # damp the ding
        y.append(d + dong[i])
    fade = int(FADE_SECONDS * SR)
    for k in range(fade):
        y[N - fade + k] *= (1 - k / fade) ** 2
    mean = sum(y) / N
    y = [v - mean for v in y]
    scale = PEAK / max(abs(v) for v in y)
    pcm = b"".join(struct.pack("<h", int(v * scale * 32767)) for v in y)
    with wave.open(sys.argv[1], "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(SR)
        w.writeframes(pcm)
    print(f"wrote {sys.argv[1]}: {N} samples, {SECONDS} s, 24 kHz 16-bit mono")


if __name__ == "__main__":
    main()
