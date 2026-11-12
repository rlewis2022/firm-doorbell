#!/usr/bin/env python3
"""Measure press-to-sound times from an RT-01 recording (budget B-03).

Part of Real-tier procedure RT-01 (tests/real/RT-01-press-to-sound.md), approved as its measurement
method in DL-026. Standard library only (absolute A-08). Supplemental: the firmware and the gate don't
run it.

Input: a WAV recording (PCM 16-bit, mono or stereo, any sample rate) of button presses on the Doorbell,
with the microphone close to the button and the speaker. Convert other formats to WAV first (for
example, Audacity: File > Export Audio > WAV).

For each press it finds:
  - the click: a sharp spike in the high-frequency band (above 2 kHz), and its start;
  - the chime: where the 400-1400 Hz band (the bell's range) rises out of the background noise.
A spike counts as a click only if it's sharp: at least 90% of its high-band energy in a 60 ms window
falls within 2 ms of its peak (button clicks measured 97-99%; other sounds far less).
A click counts as a press only if a chime follows it (the chime band is at least 4x louder 250-450 ms
after the click than in the 300 ms before it). Other sounds (handling noise, release clicks during a
chime) are reported as excluded. If another sharp click falls in the 500 ms before a chime, it isn't
clear which click started it: that press is flagged AMBIGUOUS for a person to check, and kept out of
the verdict.

Usage:  python3 tools/measure_press_to_sound.py recording.wav
Prints one line per press (time in the file, click-to-chime in ms), then max, median and the B-03 verdict.
"""
import math
import statistics
import struct
import sys
import wave

B03_LIMIT_MS = 300.0
B03_WARNING_MS = 240.0

CLICK_HP_HZ = 2000.0        # click detection band
CHIME_LO_HZ, CHIME_HI_HZ = 400.0, 1400.0
CLICK_FACTOR = 50.0         # a click is this many times the high-band noise level
CLICK_SHARPNESS = 0.9       # a click puts at least this share of its energy (60 ms window) within +-2 ms
TONE_FACTOR = 4.0           # the chime band must rise at least this much for a press
SKIP_AFTER_PRESS_S = 2.0    # one press per chime (the dong comes 0.62 s after the ding)


def read_wav(path):
    with wave.open(path, "rb") as w:
        if w.getsampwidth() != 2:
            sys.exit("Need 16-bit PCM WAV.")
        ch, sr, n = w.getnchannels(), w.getframerate(), w.getnframes()
        raw = w.readframes(n)
    vals = struct.unpack("<%dh" % (len(raw) // 2), raw)
    if ch == 1:
        return [v / 32768.0 for v in vals], sr
    return [sum(vals[i:i + ch]) / (ch * 32768.0) for i in range(0, len(vals), ch)], sr


def biquad(x, b0, b1, b2, a1, a2):
    y = [0.0] * len(x)
    x1 = x2 = y1 = y2 = 0.0
    for i, xi in enumerate(x):
        yi = b0 * xi + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2
        x2, x1, y2, y1 = x1, xi, y1, yi
        y[i] = yi
    return y


def rbj(kind, f0, sr, q=0.7071):
    w0 = 2 * math.pi * f0 / sr
    alpha = math.sin(w0) / (2 * q)
    c = math.cos(w0)
    if kind == "hp":
        b = [(1 + c) / 2, -(1 + c), (1 + c) / 2]
    else:  # "lp"
        b = [(1 - c) / 2, 1 - c, (1 - c) / 2]
    a0 = 1 + alpha
    return [b[0] / a0, b[1] / a0, b[2] / a0, -2 * c / a0, (1 - alpha) / a0]


def zero_phase(x, stages):
    """Filter forward and backward so the filter adds no delay (timing must not shift)."""
    for s in stages:
        x = biquad(x, *s)
    x.reverse()
    for s in stages:
        x = biquad(x, *s)
    x.reverse()
    return x


def rms_envelope(x, n):
    """Centered moving RMS over n samples."""
    out = [0.0] * len(x)
    acc = 0.0
    sq = [v * v for v in x]
    half = n // 2
    for i in range(len(x) + half):
        if i < len(x):
            acc += sq[i]
        if i - n >= 0:
            acc -= sq[i - n]
        j = i - half
        if 0 <= j < len(x):
            out[j] = math.sqrt(max(acc, 0.0) / n)
    return out


def sharp_click_at(hp, h, ms):
    e_near = sum(hp[k] ** 2 for k in range(h - ms(2), h + ms(2)))
    e_wide = sum(hp[k] ** 2 for k in range(h - ms(30), h + ms(30))) + 1e-18
    return e_near / e_wide >= CLICK_SHARPNESS


def other_clicks(hp, start, end, skip, noise, ms):
    """Sharp clicks in [start, end), ignoring any within 10 ms of `skip`."""
    found, k = [], start
    while k < end:
        if abs(hp[k]) >= CLICK_FACTOR * noise and abs(k - skip) > ms(10):
            h = max(range(k, min(k + ms(10), end)), key=lambda j: abs(hp[j]))
            if sharp_click_at(hp, h, ms):
                found.append(h)
            k = h + ms(10)
        else:
            k += 1
    return found


def median(v):
    return statistics.median(v) if v else 0.0


def main():
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    y, sr = read_wav(sys.argv[1])
    ms = lambda t: int(t * sr / 1000.0)
    hp = zero_phase(y, [rbj("hp", CLICK_HP_HZ, sr)] * 2)
    band = zero_phase(y, [rbj("hp", CHIME_LO_HZ, sr)] * 2 + [rbj("lp", CHIME_HI_HZ, sr)] * 2)
    env = rms_envelope(band, ms(5))
    hp_noise = median([abs(v) for v in hp[::7]]) * 1.4826 + 1e-9   # robust noise level

    presses, excluded = [], 0
    i, n = ms(400), len(y) - ms(500)
    while i < n:
        if abs(hp[i]) < CLICK_FACTOR * hp_noise:
            i += 1
            continue
        # the click: the biggest raw-signal spike within the next 20 ms
        seg = range(i, min(i + ms(20), len(y)))
        c = max(seg, key=lambda k: abs(y[k]))
        h = max(seg, key=lambda k: abs(hp[k]))
        if not sharp_click_at(hp, h, ms):
            excluded += 1
            i = h + ms(100)
            continue
        pre = median(env[c - ms(300):c - ms(50)])
        tone = median(env[c + ms(250):c + ms(450)])
        if tone < TONE_FACTOR * (pre + 1e-12):
            excluded += 1
            i = c + ms(100)
            continue
        # click start: back to 20% of its peak, then to where it leaves the noise (within 5 ms)
        peak = abs(y[c])
        c0 = c
        while c0 > c - ms(20) and abs(y[c0 - 1]) > 0.2 * peak:
            c0 -= 1
        raw_noise = median([abs(v) for v in y[c - ms(800):c - ms(600)]]) * 1.4826 + 1e-9
        lim = c0 - ms(5)
        while c0 > lim and abs(y[c0 - 1]) > 10 * raw_noise:
            c0 -= 1
        # chime start: first 50% crossing after click + 40 ms, walked back to pre + 10% of the rise
        rise = tone - pre
        t = c + ms(40)
        while t < len(env) and env[t] <= pre + 0.5 * rise:
            t += 1
        while t > c + ms(40) and env[t - 1] > pre + 0.1 * rise:
            t -= 1
        # ambiguous if another sharp click falls between this click and the chime, or in the
        # 500 ms before the chime: then it isn't clear which click started it
        extra = other_clicks(hp, max(t - ms(500), 0), t, h, hp_noise, ms)
        presses.append((c0 / sr, (t - c0) * 1000.0 / sr, len(extra)))
        i = c + ms(SKIP_AFTER_PRESS_S * 1000)

    if not presses:
        sys.exit("No presses found (no click followed by a chime).")
    print(f"File: {sys.argv[1]}  ({len(y) / sr:.1f} s, {sr} Hz)")
    print("Press  At (m:ss.s)  Click-to-chime (ms)")
    for k, (at, d, x) in enumerate(presses, 1):
        note = f"   AMBIGUOUS: {x} other click(s) just before the chime; check by hand" if x else ""
        print(f"{k:>5}  {int(at // 60)}:{at % 60:04.1f}      {d:6.1f}{note}")
    times = [d for _, d, x in presses if not x]
    ambiguous = sum(1 for p in presses if p[2])
    if not times:
        sys.exit("Every press is ambiguous; measure by hand.")
    mx, md = max(times), statistics.median(times)
    print(f"Clean presses: {len(times)} · ambiguous: {ambiguous} · excluded sounds: {excluded} · "
          f"max {mx:.1f} ms · median {md:.1f} ms (clean presses)")
    if ambiguous:
        print("Ambiguous presses aren't in the verdict. A person must look at each one; if it was a "
              "real press, its time counts.")
    if len(times) < 10:
        verdict = "INCOMPLETE: RT-01 needs at least 10 clean presses"
    elif mx > B03_LIMIT_MS:
        verdict = f"FAIL: over the B-03 limit ({B03_LIMIT_MS:.0f} ms)"
    elif ambiguous:
        verdict = (f"NOT YET: the clean presses are within the limit, but {ambiguous} ambiguous "
                   "press(es) must be checked by hand before RT-01 can be recorded as a pass")
    elif mx > B03_WARNING_MS:
        verdict = f"PASS, warning line crossed (> {B03_WARNING_MS:.0f} ms): record it for the Verifier"
    else:
        verdict = "PASS"
    print("B-03:", verdict)


if __name__ == "__main__":
    main()
