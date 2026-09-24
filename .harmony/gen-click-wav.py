#!/usr/bin/env python3
"""gen-click-wav.py -- s-rta-0923 step3 lane S3-G. python3 stdlib only (no
numpy/PIL) so probe-step3.sh can generate the click WAV without the .venv.

Writes a deterministic click-track WAV: silence with a short, enveloped
burst (default 5ms: a fast linear attack into an exponentially-decaying
1kHz-tone + noise mix) every `--interval` frames -- the D10.3 T1 alignment
signal (24000 frames @ 48kHz = 0.5s = 120 BPM equivalent, plan s-rta-0923
step3 section 4). The BURST ONSET (its very first sample, before the
attack ramp) lands exactly on the grid (i.e. at frame i*interval) so the T2
alignment math in probe-step3.sh stays valid.

s-rta-0924 (Harmony, diagnosis-driven fix): the previous generator wrote a
single full-scale IMPULSE sample per click. aubio's spectral-flux onset
detector (OnsetDetector.cpp: threshold 0.3, minioi 50ms, 2048-sample Hann
window) integrates energy over a windowed/hopped analysis (512-sample hop,
2048-sample window @ 48kHz) and never fires on a 1-sample impulse -- so
take.markers stayed empty and the T2 alignment check was permanently NA.
An enveloped multi-millisecond burst gives the windowed onset detector
enough sustained energy and spectral-flux rise to fire reliably.
probe-step3.sh's own click-peak detector (the T2 alignment block, section 9)
finds the ONSET of each burst too -- the first sample whose magnitude
crosses half full-scale, with same-cluster hits within 100 samples
collapsed to that first (onset) sample -- so keep the amplitude comfortably
above that threshold once the attack ramp completes; do not change that
detector's threshold/clustering logic to match this file, it already
detects onset, not peak.

Usage: gen-click-wav.py [output.wav] [--duration-s N] [--rate N]
                         [--interval N] [--channels N] [--amplitude N]
                         [--burst-ms N] [--attack-ms N] [--freq-hz N]
                         [--tone-mix N] [--noise-mix N] [--decay-k N]
                         [--seed N]
Defaults match the plan exactly for the click grid: 120s, 48000 Hz, stereo,
a burst onset every 24000 frames, amplitude 30000 (comfortably below int16
full scale to avoid any downstream clipping in the tap's re-encode). Burst
shape defaults: 5ms total, 0.5ms linear attack, 1kHz tone + noise (70/30
mix), exponential decay (k=5.0, so the tail is down to exp(-5)~=0.7% of
peak by the end of the burst). `--seed` keeps the noise component
deterministic across runs (default 0).
"""
import argparse
import math
import random
import struct
import wave


def main() -> int:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("output", nargs="?", default="/tmp/click_48k.wav")
    p.add_argument("--duration-s", type=float, default=120.0)
    p.add_argument("--rate", type=int, default=48000)
    p.add_argument("--interval", type=int, default=24000)
    p.add_argument("--channels", type=int, default=2)
    p.add_argument("--amplitude", type=int, default=30000)
    p.add_argument("--burst-ms", type=float, default=5.0)
    p.add_argument("--attack-ms", type=float, default=0.5)
    p.add_argument("--freq-hz", type=float, default=1000.0)
    p.add_argument("--tone-mix", type=float, default=0.7)
    p.add_argument("--noise-mix", type=float, default=0.3)
    p.add_argument("--decay-k", type=float, default=5.0)
    p.add_argument("--seed", type=int, default=0)
    # s-rta-0924: a continuous noise floor between bursts. aubio's onset
    # detector confirms a peak one hop late and drops it if that hop is TRUE
    # digital silence (-inf dB) -- with an all-zero gap no onset ever fires
    # (diagnosed: 0 onsets at any threshold/silence setting; -60..-40 dBFS
    # floor restores 10/10). Real mic/file audio always has a floor.
    p.add_argument("--floor-dbfs", type=float, default=-50.0)
    args = p.parse_args()

    total_frames = int(round(args.duration_s * args.rate))
    burst_frames = max(1, int(round(args.burst_ms / 1000.0 * args.rate)))
    # Never let a burst overrun the next grid point -- keeps every click's
    # onset exactly on the grid with no overlap between consecutive bursts.
    burst_frames = min(burst_frames, args.interval)
    attack_frames = max(1, min(burst_frames, int(round(args.attack_ms / 1000.0 * args.rate))))
    decay_frames = max(1, burst_frames - attack_frames)

    rng = random.Random(args.seed)
    floor_amp = 32767.0 * (10.0 ** (args.floor_dbfs / 20.0))
    silence_frame = struct.pack("<" + "h" * args.channels, *([0] * args.channels))

    def envelope(k: int) -> float:
        if k < attack_frames:
            return k / attack_frames
        t = (k - attack_frames) / decay_frames
        return math.exp(-args.decay_k * t)

    def clamp_int16(v: float) -> int:
        return max(-32768, min(32767, int(round(v))))

    n_clicks = 0
    with wave.open(args.output, "wb") as w:
        w.setnchannels(args.channels)
        w.setsampwidth(2)  # int16
        w.setframerate(args.rate)
        i = 0
        while i < total_frames:
            if i % args.interval == 0:
                this_burst = min(burst_frames, total_frames - i)
                for k in range(this_burst):
                    env = envelope(k)
                    tone = math.sin(2.0 * math.pi * args.freq_hz * k / args.rate)
                    noise = rng.uniform(-1.0, 1.0)
                    s = env * args.amplitude * (args.tone_mix * tone + args.noise_mix * noise)
                    sample = clamp_int16(s)
                    w.writeframesraw(struct.pack("<" + "h" * args.channels, *([sample] * args.channels)))
                n_clicks += 1
                i += this_burst
            else:
                fv = int(round(rng.uniform(-1.0, 1.0) * floor_amp))
                w.writeframesraw(struct.pack("<" + "h" * args.channels, *([fv] * args.channels)))
                i += 1

    print(f"wrote {args.output}: {total_frames} frames @ {args.rate} Hz, "
          f"{args.channels} ch, {n_clicks} bursts ({burst_frames} frames = "
          f"{burst_frames / args.rate * 1000.0:.2f}ms each, onset-aligned to "
          f"the grid) every {args.interval} frames "
          f"({args.interval / args.rate:.3f}s = {60.0 * args.rate / args.interval:.1f} BPM equivalent)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
